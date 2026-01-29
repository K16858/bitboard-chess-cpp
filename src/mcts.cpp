#include "mcts.hpp"
#include "movegen.hpp"
#include <cmath>
#include <random>

namespace {
    const double UCB_C = 1.4142135623730950488;  // sqrt(2)

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
    MCTSResult out;
    out.rootValue = 0.0;
    out.rootVisits = 0;
    if (iterations <= 0) return out;

    MCTSNode* root = new MCTSNode();
    root->parent = nullptr;
    root->N = 0;
    root->W = 0.0;
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
                for (const Move& m : moves) {
                    MCTSNode* c = new MCTSNode();
                    c->move_from_parent = m;
                    c->parent = node;
                    c->N = 0;
                    c->W = 0.0;
                    node->children.push_back(c);
                }
                std::uniform_int_distribution<std::size_t> dist(0, node->children.size() - 1);
                std::size_t idx = dist(gen);
                MCTSNode* leaf = node->children[idx];
                board.MakeMove(leaf->move_from_parent);
                double value = resultToValue(MoveGen::DoRandomPlayout(board, gen), rootWhite);
                for (MCTSNode* p = leaf; p != nullptr; p = p->parent) {
                    p->N++;
                    p->W += value;
                }
                break;
            }

            MCTSNode* best = nullptr;
            double bestScore = -1e99;
            int parentN = node->N;
            for (MCTSNode* c : node->children) {
                double score;
                if (c->N == 0)
                    score = 1e99;
                else
                    score = c->W / c->N + UCB_C * std::sqrt(std::log(static_cast<double>(parentN + 1)) / c->N);
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
    MCTSResult res = RunMCTS(root, iterations, gen);
    if (res.visits.empty()) return Move();
    const auto* best = &res.visits[0];
    for (const auto& p : res.visits)
        if (p.second > best->second) best = &p;
    return best->first;
}
