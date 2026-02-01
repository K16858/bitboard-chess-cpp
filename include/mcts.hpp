#ifndef MCTS_HPP
#define MCTS_HPP

#include "board.hpp"
#include "move.hpp"
#include <functional>
#include <random>
#include <string>
#include <utility>
#include <vector>

struct MCTSNode {
    Move move_from_parent;
    MCTSNode* parent;
    std::vector<MCTSNode*> children;
    int N;
    double W;
    double P;  // prior (P(s,a)); 未設定時は一様
    int N_virtual = 0;  // バッチ用: 選択中ワーカー数。UCB で N + N_virtual として使用し、並列ワーカーが同じ子を選ばないようにする
};

// RunMCTSの戻り値
struct MCTSResult {
    std::vector<std::pair<Move, int>> visits;
    double rootValue;
    int rootVisits;
};

/// prior/value を外から注入するオプション。空なら従来どおり一様 prior + DoRandomPlayout。
/// batch_prior_fn / batch_value_fn が両方セットされている場合はバッチモードで呼び出し回数を削減。
struct MCTSOptions {
    std::function<std::vector<double>(const Board&, const std::vector<Move>&)> prior_fn;
    std::function<double(const Board&)> value_fn;
    /// バッチモード用: (fen_list, uci_list_per_fen) -> prior_list (各要素は合法手数と同じ長さ)
    std::function<std::vector<std::vector<double>>(
        const std::vector<std::string>& fens,
        const std::vector<std::vector<std::string>>& uci_list_per_fen)> batch_prior_fn;
    /// バッチモード用: fen_list -> value_list (各要素は [-1,1])
    std::function<std::vector<double>(const std::vector<std::string>& fens)> batch_value_fn;
    int batch_size = 32;
    double c_puct = 1.4142135623730950488;  // sqrt(2)
    /// ルートの prior に加えるディリクレノイズ。0.0 なら無効
    double dirichlet_alpha = 0.0;
    /// ルートでの混合率: (1-epsilon)*prior + epsilon*dirichlet
    double dirichlet_epsilon = 0.25;
};

MCTSResult RunMCTS(const Board& root, int iterations, std::mt19937& gen);
MCTSResult RunMCTS(const Board& root, int iterations, std::mt19937& gen, const MCTSOptions& options);

/// MCTS で最善手を1手返す（訪問数が最大の手）。合法手がない場合は未使用の Move を返す。
Move GetBestMoveMCTS(const Board& root, int iterations, std::mt19937& gen);
Move GetBestMoveMCTS(const Board& root, int iterations, std::mt19937& gen, const MCTSOptions& options);

#endif
