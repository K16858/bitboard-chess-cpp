#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "board.h"
#include <functional>

class MoveGen {
private:
    static U64 pawnMoves[64];
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
    static U64 GetPawnMoves(Square square);
    static U64 GetRookMoves(Square square);
    static U64 GetBishopMoves(Square square);
    static U64 GetKnightMoves(Square square);
    static U64 GetQueenMoves(Square square);
    static U64 GetKingMoves(Square square);
};

#endif