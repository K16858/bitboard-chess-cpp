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
};

PYBIND11_MODULE(chess_engine, m) {
    m.doc() = "Chess engine with MCTS (pybind11 binding)";

    m.def("init", &MoveGen::Init, "Initialize move generator tables. Call once before using Board or run_mcts.");

    m.def("run_mcts", [](BoardWrapper& bw, int iterations, unsigned int seed) {
        std::mt19937 gen(seed);
        MCTSResult res = RunMCTS(bw.board_, iterations, gen);
        std::vector<int> visits;
        visits.reserve(res.visits.size());
        for (const auto& p : res.visits) visits.push_back(p.second);
        return py::make_tuple(visits, res.rootValue, res.rootVisits);
    }, py::arg("board"), py::arg("iterations"), py::arg("seed"),
       "Run MCTS on board. Returns (visits, root_value, root_visits). visits[i] matches legal_moves()[i].");

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
        .def("fen", &BoardWrapper::fen);
}
