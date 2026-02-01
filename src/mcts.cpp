#include "mcts.hpp"
#include "movegen.hpp"
#include "move.hpp"
#include <cmath>
#include <map>
#include <random>
#include <set>
#include <string>
#include <vector>

namespace {
    static double resultToValue(GameResult r, bool rootWhite) {
        if (r == GameResult::Draw) return 0.0;
        if (r == GameResult::WhiteWin) return rootWhite ? 1.0 : -1.0;
        if (r == GameResult::BlackWin) return rootWhite ? -1.0 : 1.0;
        return 0.0;
    }

    static void deleteTree(MCTSNode* n) {
        if (!n) return;
        for (MCTSNode* c : n->children)
            deleteTree(c);
        delete n;
    }

    static std::string moveToUci(const Move& m) {
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

    static std::vector<std::string> movesToUci(const std::vector<Move>& moves) {
        std::vector<std::string> out;
        out.reserve(moves.size());
        for (const Move& m : moves) out.push_back(moveToUci(m));
        return out;
    }

    enum WorkerState { RUN, NEED_PRIOR, NEED_VALUE };

    struct Worker {
        Board board;
        MCTSNode* node;
        WorkerState state = RUN;
        std::vector<Move> moves;
        MCTSNode* leaf_node = nullptr;
    };
}

MCTSResult RunMCTS(const Board& rootBoard, int iterations, std::mt19937& gen) {
    return RunMCTS(rootBoard, iterations, gen, MCTSOptions{});
}

static MCTSResult RunMCTSBatch(const Board& rootBoard, int iterations, const MCTSOptions& options);

MCTSResult RunMCTS(const Board& rootBoard, int iterations, std::mt19937& gen, const MCTSOptions& options) {
    MCTSResult out;
    out.rootValue = 0.0;
    out.rootVisits = 0;
    if (iterations <= 0) return out;

    if (options.batch_prior_fn && options.batch_value_fn) {
        return RunMCTSBatch(rootBoard, iterations, options);
    }

    const double c_puct = options.c_puct;
    MCTSNode* root = new MCTSNode();
    root->parent = nullptr;
    root->N = 0;
    root->W = 0.0;
    root->P = 0.0;
    bool rootWhite = rootBoard.GetWhiteToMove();

    for (int iter = 0; iter < iterations; iter++) {
        Board board = rootBoard;
        MCTSNode* node = root;

        while (true) {
            if (node->children.empty()) {
                std::vector<Move> moves;
                MoveGen::GenerateLegalMoves(board, moves);
                if (moves.empty()) {
                    double value = resultToValue(MoveGen::GetGameResult(board), rootWhite);
                    for (MCTSNode* p = node; p != nullptr; p = p->parent) {
                        p->N++;
                        p->W += value;
                    }
                    break;
                }
                std::vector<double> priors;
                if (options.prior_fn) {
                    priors = options.prior_fn(board, moves);
                    if (priors.size() != moves.size()) priors.clear();
                }
                double sumP = 0.0;
                if (!priors.empty()) {
                    for (double x : priors) sumP += (x > 0.0 ? x : 0.0);
                }
                const double uniformP = 1.0 / static_cast<double>(moves.size());
                for (std::size_t i = 0; i < moves.size(); i++) {
                    MCTSNode* c = new MCTSNode();
                    c->move_from_parent = moves[i];
                    c->parent = node;
                    c->N = 0;
                    c->W = 0.0;
                    if (sumP > 0.0 && i < priors.size() && priors[i] > 0.0)
                        c->P = priors[i] / sumP;
                    else
                        c->P = uniformP;
                    node->children.push_back(c);
                }
                MCTSNode* best = nullptr;
                double bestScore = -1e99;
                int parentN = node->N;
                for (MCTSNode* c : node->children) {
                    double score = c_puct * c->P * std::sqrt(static_cast<double>(parentN + 1)) / (1.0 + c->N);
                    if (c->N > 0)
                        score += c->W / c->N;
                    if (score > bestScore) {
                        bestScore = score;
                        best = c;
                    }
                }
                if (!best) break;
                board.MakeMove(best->move_from_parent);
                double value;
                if (options.value_fn) {
                    value = options.value_fn(board);
                } else {
                    value = resultToValue(MoveGen::DoRandomPlayout(board, gen), rootWhite);
                }
                double sign = 1.0;
                for (MCTSNode* p = best; p != nullptr; p = p->parent) {
                    p->N++;
                    p->W += sign * value;
                    sign = -sign;
                }
                break;
            }

            MCTSNode* best = nullptr;
            double bestScore = -1e99;
            int parentN = node->N;
            for (MCTSNode* c : node->children) {
                double score = c_puct * c->P * std::sqrt(static_cast<double>(parentN + 1)) / (1.0 + c->N);
                if (c->N > 0)
                    score += c->W / c->N;
                if (score > bestScore) {
                    bestScore = score;
                    best = c;
                }
            }
            if (!best) break;
            board.MakeMove(best->move_from_parent);
            node = best;
        }
    }

    out.rootVisits = root->N;
    out.rootValue = (root->N > 0) ? (root->W / root->N) : 0.0;
    for (MCTSNode* c : root->children)
        out.visits.push_back({c->move_from_parent, c->N});

    deleteTree(root);
    return out;
}

static MCTSResult RunMCTSBatch(const Board& rootBoard, int iterations, const MCTSOptions& options) {
    const double c_puct = options.c_puct;
    const int W = std::max(1, std::min(options.batch_size, 1024));
    const bool rootWhite = rootBoard.GetWhiteToMove();

    MCTSNode* root = new MCTSNode();
    root->parent = nullptr;
    root->N = 0;
    root->W = 0.0;
    root->P = 0.0;

    std::vector<Worker> workers(static_cast<std::size_t>(W));
    for (int i = 0; i < W; i++) {
        workers[i].board = rootBoard;
        workers[i].node = root;
        workers[i].state = RUN;
    }

    int completed = 0;

    while (completed < iterations) {
        // --- Flush prior batch ---
        std::map<std::string, std::vector<std::pair<MCTSNode*, std::vector<Move>>>> priorByFen;
        for (std::size_t i = 0; i < workers.size(); i++) {
            Worker& w = workers[i];
            if (w.state != NEED_PRIOR) continue;
            std::string fen = w.board.GetFen();
            priorByFen[fen].push_back({w.node, w.moves});
        }
        if (!priorByFen.empty()) {
            std::vector<std::string> fens;
            std::vector<std::vector<std::string>> uciPerFen;
            fens.reserve(priorByFen.size());
            uciPerFen.reserve(priorByFen.size());
            for (const auto& kv : priorByFen) {
                fens.push_back(kv.first);
                uciPerFen.push_back(movesToUci(kv.second.front().second));
            }
            std::vector<std::vector<double>> priorResults = options.batch_prior_fn(fens, uciPerFen);
            if (priorResults.size() != fens.size()) priorResults.clear();

            for (std::size_t fi = 0; fi < fens.size(); fi++) {
                const std::vector<double>& priors = (fi < priorResults.size()) ? priorResults[fi] : std::vector<double>();
                const auto& nodesAndMoves = priorByFen[fens[fi]];
                const std::vector<Move>& moves = nodesAndMoves.front().second;
                double sumP = 0.0;
                if (priors.size() == moves.size()) {
                    for (double x : priors) sumP += (x > 0.0 ? x : 0.0);
                }
                const double uniformP = 1.0 / static_cast<double>(moves.size());

                std::set<MCTSNode*> expanded;
                for (const auto& np : nodesAndMoves) {
                    MCTSNode* node = np.first;
                    if (expanded.count(node)) continue;
                    expanded.insert(node);
                    const std::vector<Move>& mov = np.second;
                    for (std::size_t i = 0; i < mov.size(); i++) {
                        MCTSNode* c = new MCTSNode();
                        c->move_from_parent = mov[i];
                        c->parent = node;
                        c->N = 0;
                        c->W = 0.0;
                        if (sumP > 0.0 && i < priors.size() && priors[i] > 0.0)
                            c->P = priors[i] / sumP;
                        else
                            c->P = uniformP;
                        node->children.push_back(c);
                    }
                }
            }

            for (std::size_t i = 0; i < workers.size(); i++) {
                Worker& w = workers[i];
                if (w.state != NEED_PRIOR) continue;
                MCTSNode* node = w.node;
                int parentN = node->N;
                MCTSNode* best = nullptr;
                double bestScore = -1e99;
                for (MCTSNode* c : node->children) {
                    double score = c_puct * c->P * std::sqrt(static_cast<double>(parentN + 1)) / (1.0 + c->N);
                    if (c->N > 0) score += c->W / c->N;
                    if (score > bestScore) { bestScore = score; best = c; }
                }
                if (!best) { w.state = RUN; continue; }
                w.board.MakeMove(best->move_from_parent);
                w.node = best;
                w.leaf_node = best;
                w.state = NEED_VALUE;
            }
        }

        // --- Flush value batch ---
        std::map<std::string, std::vector<std::size_t>> valueWorkerIndices;
        for (std::size_t i = 0; i < workers.size(); i++) {
            if (workers[i].state != NEED_VALUE) continue;
            valueWorkerIndices[workers[i].board.GetFen()].push_back(i);
        }
        if (!valueWorkerIndices.empty()) {
            std::vector<std::string> fens;
            fens.reserve(valueWorkerIndices.size());
            for (const auto& kv : valueWorkerIndices) fens.push_back(kv.first);
            std::vector<double> values = options.batch_value_fn(fens);
            if (values.size() != fens.size()) values.assign(fens.size(), 0.0);

            for (std::size_t fi = 0; fi < fens.size(); fi++) {
                double value = (fi < values.size()) ? values[fi] : 0.0;
                for (std::size_t idx : valueWorkerIndices[fens[fi]]) {
                    Worker& w = workers[idx];
                    double sign = 1.0;
                    for (MCTSNode* p = w.leaf_node; p != nullptr; p = p->parent) {
                        p->N++;
                        p->W += sign * value;
                        sign = -sign;
                    }
                    completed++;
                    w.board = rootBoard;
                    w.node = root;
                    w.state = RUN;
                    w.leaf_node = nullptr;
                }
            }
        }

        // --- Advance RUN workers ---
        for (std::size_t i = 0; i < workers.size(); i++) {
            Worker& w = workers[i];
            if (w.state != RUN) continue;

            if (w.node->children.empty()) {
                MoveGen::GenerateLegalMoves(w.board, w.moves);
                if (w.moves.empty()) {
                    double value = resultToValue(MoveGen::GetGameResult(w.board), rootWhite);
                    double sign = 1.0;
                    for (MCTSNode* p = w.node; p != nullptr; p = p->parent) {
                        p->N++;
                        p->W += sign * value;
                        sign = -sign;
                    }
                    completed++;
                    w.board = rootBoard;
                    w.node = root;
                    w.state = RUN;
                    continue;
                }
                w.state = NEED_PRIOR;
                continue;
            }

            int parentN = w.node->N;
            MCTSNode* best = nullptr;
            double bestScore = -1e99;
            for (MCTSNode* c : w.node->children) {
                double score = c_puct * c->P * std::sqrt(static_cast<double>(parentN + 1)) / (1.0 + c->N);
                if (c->N > 0) score += c->W / c->N;
                if (score > bestScore) { bestScore = score; best = c; }
            }
            if (!best) continue;
            w.board.MakeMove(best->move_from_parent);
            w.node = best;
            if (best->children.empty()) {
                w.leaf_node = best;
                w.state = NEED_VALUE;
            }
        }
    }

    MCTSResult out;
    out.rootVisits = root->N;
    out.rootValue = (root->N > 0) ? (root->W / root->N) : 0.0;
    for (MCTSNode* c : root->children)
        out.visits.push_back({c->move_from_parent, c->N});

    deleteTree(root);
    return out;
}

Move GetBestMoveMCTS(const Board& root, int iterations, std::mt19937& gen) {
    return GetBestMoveMCTS(root, iterations, gen, MCTSOptions{});
}

Move GetBestMoveMCTS(const Board& root, int iterations, std::mt19937& gen, const MCTSOptions& options) {
    MCTSResult res = RunMCTS(root, iterations, gen, options);
    if (res.visits.empty()) return Move();
    const auto* best = &res.visits[0];
    for (const auto& p : res.visits)
        if (p.second > best->second) best = &p;
    return best->first;
}
