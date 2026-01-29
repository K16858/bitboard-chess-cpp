#ifndef MOVEGEN_HPP
#define MOVEGEN_HPP

#include "board.hpp"
#include <functional>
#include <random>
#include <vector>

class MoveGen {
private:
    // ポーンの移動テーブル（色ごと）
    static U64 whitePawnMoves[64];      // 白ポーンの前進移動
    static U64 whitePawnCaptures[64];   // 白ポーンのキャプチャ移動
    static U64 blackPawnMoves[64];      // 黒ポーンの前進移動
    static U64 blackPawnCaptures[64];   // 黒ポーンのキャプチャ移動
    
    static U64 rookMoves[64];
    static U64 bishopMoves[64];
    static U64 knightMoves[64];
    static U64 queenMoves[64];
    static U64 kingMoves[64];
    static bool initialized;
    
    static void InitPawnMoves();
    static void InitRookMoves();
    static void InitBishopMoves();
    static void InitKnightMoves();
    static void InitQueenMoves();
    static void InitKingMoves();

    static U64 GenerateMoves(int square, const int offsets[], int numOffsets, std::function<bool(int, int)> isValidMove);
    
public:
    static void Init(); 
    // ポーンの移動（色を指定：true=白、false=黒）
    static U64 GetPawnMoves(Square square, bool isWhite);
    static U64 GetPawnCaptures(Square square, bool isWhite);
    static U64 GetRookMoves(Square square, U64 occupancy = 0);
    static U64 GetBishopMoves(Square square, U64 occupancy = 0);
    static U64 GetKnightMoves(Square square);
    static U64 GetQueenMoves(Square square, U64 occupancy = 0);
    static U64 GetKingMoves(Square square);
    static void GenerateLegalMoves(Board& board, std::vector<Move>& moves);
    /// 終局結果を返す（白勝ち=1, 黒勝ち=-1, 引き分け=0, 進行中=Ongoing）
    static GameResult GetGameResult(Board& board);
    /// ランダムプレイアウト
    static GameResult DoRandomPlayout(Board board, std::mt19937& gen);
};

#endif