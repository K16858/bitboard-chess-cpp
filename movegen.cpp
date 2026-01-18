#include "movegen.h"
#include <cstdlib>

U64 MoveGen::kingMoves[64] = {0};
U64 MoveGen::knightMoves[64] = {0};
bool MoveGen::initialized = false;

void MoveGen::InitKnightMoves() {
    int knightOffsets[8] = {17, 15, 10, 6, -6, -10, -15, -17};
    
    for (int square = 0; square < 64; square++) {
        U64 attacks = 0ULL;
        int rank = square / 8;
        int file = square % 8;
        
        for (int offset : knightOffsets) {
            int targetSquare = square + offset;
            
            if (targetSquare < 0 || targetSquare >= 64) continue;
            
            int targetRank = targetSquare / 8;
            int targetFile = targetSquare % 8;
            
            int rankDiff = abs(targetRank - rank);
            int fileDiff = abs(targetFile - file);
            
            if ((rankDiff == 2 && fileDiff == 1) || (rankDiff == 1 && fileDiff == 2)) {
                attacks |= (1ULL << targetSquare);
            }
        }
        
        knightMoves[square] = attacks;
    }
}

void MoveGen::InitKingMoves() {
}

void MoveGen::Init() {
    if (!initialized) {
        InitKnightMoves();
        InitKingMoves();
        initialized = true;
    }
}

U64 MoveGen::GetKnightMoves(Square square) {
    return knightMoves[square];
}

U64 MoveGen::GetKingMoves(Square square) {
    return kingMoves[square];
}
