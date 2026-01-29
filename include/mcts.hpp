#ifndef MCTS_HPP
#define MCTS_HPP

#include "board.hpp"
#include "move.hpp"
#include <functional>
#include <random>
#include <utility>
#include <vector>

struct MCTSNode {
    Move move_from_parent;
    MCTSNode* parent;
    std::vector<MCTSNode*> children;
    int N;
    double W;
    double P;  // prior (P(s,a)); 未設定時は一様
};

// RunMCTSの戻り値
struct MCTSResult {
    std::vector<std::pair<Move, int>> visits;
    double rootValue;
    int rootVisits;
};

/// prior/value を外から注入するオプション。空なら従来どおり一様 prior + DoRandomPlayout。
struct MCTSOptions {
    std::function<std::vector<double>(const Board&, const std::vector<Move>&)> prior_fn;
    std::function<double(const Board&)> value_fn;
    double c_puct = 1.4142135623730950488;  // sqrt(2)
};

MCTSResult RunMCTS(const Board& root, int iterations, std::mt19937& gen);
MCTSResult RunMCTS(const Board& root, int iterations, std::mt19937& gen, const MCTSOptions& options);

/// MCTS で最善手を1手返す（訪問数が最大の手）。合法手がない場合は未使用の Move を返す。
Move GetBestMoveMCTS(const Board& root, int iterations, std::mt19937& gen);
Move GetBestMoveMCTS(const Board& root, int iterations, std::mt19937& gen, const MCTSOptions& options);

#endif
