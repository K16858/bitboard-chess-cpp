#ifndef ZOBRIST_H
#define ZOBRIST_H

#include "bitboard.h"
#include "move.h"

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
