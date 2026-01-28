#ifndef ZOBRIST_HPP
#define ZOBRIST_HPP

#include "bitboard.hpp"

/// Zobrist ハッシュ用テーブル
class Zobrist {
public:
    static void Init();
    static U64 GetPieceKey(Square sq, int pieceType, bool isWhite);
    static U64 GetSideKey();
private:
    static U64 table[64][12];
    static U64 sideKey;
    static bool initialized;
};

#endif
