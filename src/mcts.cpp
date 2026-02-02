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
    /// ルート用: prior にディリクレノイズを混合。alpha<=0 のときは prior をそのまま返す。
    void applyDirichletToPriors(std::vector<double>& priors, double alpha, double epsilon, std::mt19937& gen) {
        if (alpha <= 0.0 || priors.empty()) return;
        const std::size_t K = priors.size();
        std::gamma_distribution<double> gamma(alpha, 1.0);
        std::vector<double> noise(K);
        double sumN = 0.0;
        for (std::size_t i = 0; i < K; i++) {
            noise[i] = std::max(gamma(gen), 1e-10);
            sumN += noise[i];
        }
        if (sumN <= 0.0) return;
        double sumP = 0.0;
        for (std::size_t i = 0; i < K; i++) {
            noise[i] /= sumN;
            priors[i] = (1.0 - epsilon) * priors[i] + epsilon * noise[i];
            if (priors[i] > 0.0) sumP += priors[i];
        }
        if (sumP <= 0.0) return;
        for (std::size_t i = 0; i < K; i++)
            priors[i] /= sumP;
    }
}

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

    enum WorkerState { RUN, NEED_EVAL };  // NEED_EVAL: リーフ到達。同一局面で Prior+Value 取得 → バックプロパ → 展開 → 1手進める

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

static MCTSResult RunMCTSBatch(const Board& rootBoard, int iterations, std::mt19937& gen, const MCTSOptions& options);

MCTSResult RunMCTS(const Board& rootBoard, int iterations, std::mt19937& gen, const MCTSOptions& options) {
    MCTSResult out;
    out.rootValue = 0.0;
    out.rootVisits = 0;
    if (iterations <= 0) return out;

    if (options.batch_prior_fn && options.batch_value_fn) {
        return RunMCTSBatch(rootBoard, iterations, gen, options);
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
                std::vector<double> p(moves.size());
                for (std::size_t i = 0; i < moves.size(); i++) {
                    if (sumP > 0.0 && i < priors.size() && priors[i] > 0.0)
                        p[i] = priors[i] / sumP;
                    else
                        p[i] = uniformP;
                }
                const bool isRoot = (node->parent == nullptr);
                if (isRoot && options.dirichlet_alpha > 0.0)
                    applyDirichletToPriors(p, options.dirichlet_alpha, options.dirichlet_epsilon, gen);
                for (std::size_t i = 0; i < moves.size(); i++) {
                    MCTSNode* c = new MCTSNode();
                    c->move_from_parent = moves[i];
                    c->parent = node;
                    c->N = 0;
                    c->W = 0.0;
                    c->P = p[i];
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

static MCTSResult RunMCTSBatch(const Board& rootBoard, int iterations, std::mt19937& gen, const MCTSOptions& options) {
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
        // --- Flush Eval (AlphaZero-style: same FEN for prior+value, then backprop → expand → move) ---
        using EvalEntry = std::pair<std::size_t, std::pair<MCTSNode*, std::vector<Move>>>;
        std::map<std::string, std::vector<EvalEntry>> evalByFen;
        for (std::size_t i = 0; i < workers.size(); i++) {
            Worker& w = workers[i];
            if (w.state != NEED_EVAL) continue;
            std::string fen = w.board.GetFen();
            evalByFen[fen].push_back({i, {w.node, w.moves}});
        }
        if (!evalByFen.empty()) {
            std::vector<std::string> fens;
            std::vector<std::vector<std::string>> uciPerFen;
            fens.reserve(evalByFen.size());
            uciPerFen.reserve(evalByFen.size());
            for (const auto& kv : evalByFen) {
                fens.push_back(kv.first);
                uciPerFen.push_back(movesToUci(kv.second.front().second.second));
            }
            std::vector<std::vector<double>> priorResults = options.batch_prior_fn(fens, uciPerFen);
            if (priorResults.size() != fens.size()) priorResults.clear();
            std::vector<double> values = options.batch_value_fn(fens);
            if (values.size() != fens.size()) values.assign(fens.size(), 0.0);

            for (std::size_t fi = 0; fi < fens.size(); fi++) {
                const std::vector<double>& priors = (fi < priorResults.size()) ? priorResults[fi] : std::vector<double>();
                const auto& entries = evalByFen[fens[fi]];
                const std::vector<Move>& moves = entries.front().second.second;
                double sumP = 0.0;
                if (priors.size() == moves.size()) {
                    for (double x : priors) sumP += (x > 0.0 ? x : 0.0);
                }
                const double uniformP = 1.0 / static_cast<double>(moves.size());
                std::vector<double> p(moves.size());
                for (std::size_t i = 0; i < moves.size(); i++) {
                    if (sumP > 0.0 && i < priors.size() && priors[i] > 0.0)
                        p[i] = priors[i] / sumP;
                    else
                        p[i] = uniformP;
                }
                const bool isRootForFen = (entries.front().second.first->parent == nullptr);
                if (isRootForFen && options.dirichlet_alpha > 0.0)
                    applyDirichletToPriors(p, options.dirichlet_alpha, options.dirichlet_epsilon, gen);

                std::set<MCTSNode*> expanded;
                for (const auto& e : entries) {
                    MCTSNode* node = e.second.first;
                    if (expanded.count(node)) continue;
                    expanded.insert(node);
                    const std::vector<Move>& mov = e.second.second;
                    for (std::size_t i = 0; i < mov.size(); i++) {
                        MCTSNode* c = new MCTSNode();
                        c->move_from_parent = mov[i];
                        c->parent = node;
                        c->N = 0;
                        c->W = 0.0;
                        c->P = p[i];
                        node->children.push_back(c);
                    }
                }
            }

            int remaining = std::max(0, iterations - completed);
            for (std::size_t fi = 0; fi < fens.size() && remaining > 0; fi++) {
                double value = (fi < values.size()) ? values[fi] : 0.0;
                for (const auto& e : evalByFen[fens[fi]]) {
                    if (remaining <= 0) break;
                    std::size_t idx = e.first;
                    MCTSNode* leaf = e.second.first;
                    Worker& w = workers[idx];
                    double sign = 1.0;
                    for (MCTSNode* p = leaf; p != nullptr; p = p->parent) {
                        p->N++;
                        p->W += sign * value;
                        if (p->parent != nullptr) p->N_virtual = std::max(0, p->N_virtual - 1);
                        sign = -sign;
                    }
                    completed++;
                    remaining--;
                    int parentN = leaf->N;
                    MCTSNode* best = nullptr;
                    double bestScore = -1e99;
                    for (MCTSNode* c : leaf->children) {
                        double denom = 1.0 + c->N + c->N_virtual;
                        double score = c_puct * c->P * std::sqrt(static_cast<double>(parentN + 1)) / denom;
                        if (c->N > 0) score += c->W / c->N;
                        if (score > bestScore) { bestScore = score; best = c; }
                    }
                    if (best) {
                        best->N_virtual += 1;
                        w.board.MakeMove(best->move_from_parent);
                        w.node = best;
                    }
                    w.state = RUN;
                }
            }
            for (Worker& w : workers) {
                if (w.state == NEED_EVAL) {
                    w.state = RUN;
                    w.board = rootBoard;
                    w.node = root;
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
                        if (p->parent != nullptr) p->N_virtual = std::max(0, p->N_virtual - 1);
                        sign = -sign;
                    }
                    completed++;
                    w.board = rootBoard;
                    w.node = root;
                    w.state = RUN;
                    continue;
                }
                w.state = NEED_EVAL;
                continue;
            }

            int parentN = w.node->N;
            MCTSNode* best = nullptr;
            double bestScore = -1e99;
            for (MCTSNode* c : w.node->children) {
                double denom = 1.0 + c->N + c->N_virtual;
                double score = c_puct * c->P * std::sqrt(static_cast<double>(parentN + 1)) / denom;
                if (c->N > 0) score += c->W / c->N;
                if (score > bestScore) { bestScore = score; best = c; }
            }
            if (!best) continue;
            best->N_virtual += 1;
            w.board.MakeMove(best->move_from_parent);
            w.node = best;
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
