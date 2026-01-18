#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "board.h"

// movegen.h
class MoveGen {
private:
    static U64 kingMoves[64];
    static U64 knightMoves[64];
    static bool initialized;
    
    static void InitKingMoves();
    static void InitKnightMoves();
    
public:
    static void Init();
    static U64 GetKingMoves(Square square);
    static U64 GetKnightMoves(Square square);
};

#endif