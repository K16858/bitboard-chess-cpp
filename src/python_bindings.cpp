#include "board.hpp"
#include "movegen.hpp"
#include "move.hpp"
#include "mcts.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <random>
#include <stdexcept>

namespace py = pybind11;

static std::string move_to_uci(const Move& m) {
    std::string s = SquareToStr(m.from) + SquareToStr(m.to);
    if (m.promotionPiece != NO_PIECE) {
        char c = 'q';
        if (m.promotionPiece == ROOK) c = 'r';
        else if (m.promotionPiece == BISHOP) c = 'b';
        else if (m.promotionPiece == KNIGHT) c = 'n';
        s += c;
    }
    return s;
}

static Move find_move_from_uci(const std::vector<Move>& moves, const std::string& uci) {
    for (const Move& m : moves) {
        if (move_to_uci(m) == uci) return m;
    }
    throw std::invalid_argument("move not legal: " + uci);
}

struct BoardWrapper {
    Board board_;
    Move last_move_;
    bool has_last_ = false;

    void set_fen(const std::string& fen) {
        board_.SetFromFen(fen);
        has_last_ = false;
    }

    std::vector<std::string> legal_moves() {
        std::vector<Move> moves;
        MoveGen::GenerateLegalMoves(board_, moves);
        std::vector<std::string> out;
        out.reserve(moves.size());
        for (const Move& m : moves) out.push_back(move_to_uci(m));
        return out;
    }

    void push(const std::string& uci) {
        std::vector<Move> moves;
        MoveGen::GenerateLegalMoves(board_, moves);
        last_move_ = find_move_from_uci(moves, uci);
        board_.MakeMove(last_move_);
        has_last_ = true;
    }

    void pop() {
        if (!has_last_) throw std::runtime_error("no move to undo");
        board_.UnmakeMove(last_move_);
        has_last_ = false;
    }

    int result() {
        GameResult r = MoveGen::GetGameResult(board_);
        if (r == GameResult::WhiteWin) return 1;
        if (r == GameResult::BlackWin) return -1;
        if (r == GameResult::Draw) return 0;
        return 2;  // Ongoing
    }

    bool white_to_move() const { return board_.GetWhiteToMove(); }

    std::string fen() const { return board_.GetFen(); }

    U64 get_zobrist_hash() const { return board_.GetZobristHash(); }
};

PYBIND11_MODULE(chess_engine, m) {
    m.doc() = "Chess engine with MCTS (pybind11 binding)";

    m.def("init", &MoveGen::Init, "Initialize move generator tables. Call once before using Board or run_mcts.");

    m.def("run_mcts", [](BoardWrapper& bw, int iterations, unsigned int seed,
                         py::object prior, py::object value,
                         py::object batch_eval, py::object batch_prior, py::object batch_value, int batch_size,
                         double dirichlet_alpha, double dirichlet_epsilon, double pfu_scale) {
        std::mt19937 gen(seed);
        MCTSOptions opts;
        opts.batch_size = std::max(1, std::min(batch_size, 1024));
        opts.dirichlet_alpha = dirichlet_alpha;
        opts.dirichlet_epsilon = dirichlet_epsilon;
        opts.pfu_scale = pfu_scale;

        bool use_batch_eval = (!batch_eval.is_none() && py::hasattr(batch_eval, "__call__"));
        bool use_batch_split = (!batch_prior.is_none() && py::hasattr(batch_prior, "__call__") &&
                               !batch_value.is_none() && py::hasattr(batch_value, "__call__"));
        bool use_batch = use_batch_eval || use_batch_split;

        if (use_batch_eval) {
            opts.batch_eval_fn = [batch_eval](const std::vector<std::string>& fens,
                                              const std::vector<std::vector<std::string>>& uci_list_per_fen) {
                py::gil_scoped_acquire acquire;
                py::list py_fens;
                for (const auto& f : fens) py_fens.append(py::cast(f));
                py::list py_uci_lists;
                for (const auto& u : uci_list_per_fen) py_uci_lists.append(py::cast(u));
                py::object result = batch_eval(py_fens, py_uci_lists);
                BatchEvalResult out;
                py::tuple t = result.cast<py::tuple>();
                if (t.size() < 2) return out;
                py::object prior_list = t[0];
                py::object value_list = t[1];
                for (py::handle h : prior_list) {
                    out.priors.push_back(h.cast<std::vector<double>>());
                }
                out.values = value_list.cast<std::vector<double>>();
                return out;
            };
        } else if (use_batch_split) {
            opts.batch_prior_fn = [batch_prior](const std::vector<std::string>& fens,
                                                 const std::vector<std::vector<std::string>>& uci_list_per_fen) {
                py::gil_scoped_acquire acquire;
                py::list py_fens;
                for (const auto& f : fens) py_fens.append(py::cast(f));
                py::list py_uci_lists;
                for (const auto& u : uci_list_per_fen) py_uci_lists.append(py::cast(u));
                py::object result = batch_prior(py_fens, py_uci_lists);
                std::vector<std::vector<double>> out;
                for (py::handle h : result) {
                    out.push_back(h.cast<std::vector<double>>());
                }
                return out;
            };
            opts.batch_value_fn = [batch_value](const std::vector<std::string>& fens) {
                py::gil_scoped_acquire acquire;
                py::list py_fens;
                for (const auto& f : fens) py_fens.append(py::cast(f));
                py::object result = batch_value(py_fens);
                return result.cast<std::vector<double>>();
            };
        }
        if (!use_batch) {
            if (!prior.is_none() && py::hasattr(prior, "__call__")) {
                opts.prior_fn = [prior](const Board& board, const std::vector<Move>& moves) {
                    py::gil_scoped_acquire acquire;
                    std::string fen = board.GetFen();
                    std::vector<std::string> uci;
                    uci.reserve(moves.size());
                    for (const Move& m : moves) uci.push_back(move_to_uci(m));
                    py::object result = prior(py::cast(fen), py::cast(uci));
                    return result.cast<std::vector<double>>();
                };
            }
            if (!value.is_none() && py::hasattr(value, "__call__")) {
                opts.value_fn = [value](const Board& board) {
                    py::gil_scoped_acquire acquire;
                    py::object result = value(py::cast(board.GetFen()));
                    return result.cast<double>();
                };
            }
        }

        MCTSResult res = RunMCTS(bw.board_, iterations, gen, opts);
        std::vector<std::string> uci_list;
        std::vector<int> visits;
        uci_list.reserve(res.visits.size());
        visits.reserve(res.visits.size());
        for (const auto& p : res.visits) {
            uci_list.push_back(move_to_uci(p.first));
            visits.push_back(p.second);
        }
        return py::make_tuple(uci_list, visits, res.rootValue, res.rootVisits);
    }, py::arg("board"), py::arg("iterations"), py::arg("seed"),
       py::arg("prior") = py::none(), py::arg("value") = py::none(),
       py::arg("batch_eval") = py::none(), py::arg("batch_prior") = py::none(), py::arg("batch_value") = py::none(), py::arg("batch_size") = 32,
       py::arg("dirichlet_alpha") = 0.0, py::arg("dirichlet_epsilon") = 0.25, py::arg("pfu_scale") = 0.0,
       "Run MCTS. Use batch_eval(fen_list, uci_list_per_fen) for PVNN (single inference); "
       "or batch_prior/batch_value for separate calls. "
       "dirichlet_alpha>0 adds Dirichlet noise at root (e.g. 0.3); dirichlet_epsilon mixes with prior (e.g. 0.25). "
       "pfu_scale>0 enables PFU (unvisited node initial value = parent_value - pfu_scale/sqrt(parent_N), clipped). "
       "Returns (uci_list, visits, root_value, root_visits).");

    py::class_<BoardWrapper>(m, "Board")
        .def(py::init([](py::object fen) {
            auto b = std::make_unique<BoardWrapper>();
            if (!fen.is_none()) b->set_fen(fen.cast<std::string>());
            return b.release();
        }), py::arg("fen") = py::none())
        .def("set_fen", &BoardWrapper::set_fen, py::arg("fen"))
        .def("legal_moves", &BoardWrapper::legal_moves)
        .def("push", &BoardWrapper::push, py::arg("uci"))
        .def("pop", &BoardWrapper::pop)
        .def("result", &BoardWrapper::result)
        .def_property_readonly("white_to_move", &BoardWrapper::white_to_move)
        .def("fen", &BoardWrapper::fen)
        .def("get_zobrist_hash", &BoardWrapper::get_zobrist_hash, "Return the Zobrist hash of the current position (64-bit unsigned).");
}
