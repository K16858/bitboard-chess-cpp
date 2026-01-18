#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "board.h"
#include <functional>

class MoveGen {
private:
    static U64 kingMoves[64];
    static U64 knightMoves[64];
    static bool initialized;
    
    static void InitKingMoves();
    static void InitKnightMoves();
    
    static U64 GenerateMoves(int square, const int offsets[], int numOffsets, std::function<bool(int, int)> isValidMove);
    
public:
    static void Init();
    static U64 GetKingMoves(Square square);
    static U64 GetKnightMoves(Square square);
};

#endif