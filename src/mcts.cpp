#include "mcts.hpp"
#include "movegen.hpp"
#include <cmath>
#include <random>

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
}

MCTSResult RunMCTS(const Board& rootBoard, int iterations, std::mt19937& gen) {
    return RunMCTS(rootBoard, iterations, gen, MCTSOptions{});
}

MCTSResult RunMCTS(const Board& rootBoard, int iterations, std::mt19937& gen, const MCTSOptions& options) {
    const double c_puct = options.c_puct;
    MCTSResult out;
    out.rootValue = 0.0;
    out.rootVisits = 0;
    if (iterations <= 0) return out;

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
