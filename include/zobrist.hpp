#ifndef ZOBRIST_HPP
#define ZOBRIST_HPP

#include "bitboard.hpp"

/// Zobrist ハッシュ用テーブル（千日手検出用に駒・手番・キャスリング権・アンパッサンを含む）
class Zobrist {
public:
    static void Init();
    static U64 GetPieceKey(Square sq, int pieceType, bool isWhite);
    static U64 GetSideKey();
    /// キャスリング権: idx 0=K, 1=Q, 2=k, 3=q
    static U64 GetCastlingKey(int idx);
    /// アンパッサン: epSquare は FEN の「取られるマス」16-23 or 40-47。0-15 のインデックスでキーを返す。
    static U64 GetEnPassantKey(int epKeyIndex);
private:
    static U64 table[64][12];
    static U64 sideKey;
    static U64 castlingKeys[4];
    static U64 epKeys[16];
    static bool initialized;
};

#endif
