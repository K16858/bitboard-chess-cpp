#ifndef MCTS_HPP
#define MCTS_HPP

#include "board.hpp"
#include "move.hpp"
#include <random>
#include <utility>
#include <vector>

struct MCTSNode {
    Move move_from_parent;
    MCTSNode* parent;
    std::vector<MCTSNode*> children;
    int N;
    double W;
};

// RunMCTSの戻り値
struct MCTSResult {
    std::vector<std::pair<Move, int>> visits;
    double rootValue;
    int rootVisits;
};

MCTSResult RunMCTS(const Board& root, int iterations, std::mt19937& gen);

/// MCTS で最善手を1手返す（訪問数が最大の手）。合法手がない場合は未使用の Move を返す。
Move GetBestMoveMCTS(const Board& root, int iterations, std::mt19937& gen);

#endif
