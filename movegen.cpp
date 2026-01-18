#include "movegen.h"
#include <cstdlib>
#include <functional>

U64 MoveGen::kingMoves[64] = {0};
U64 MoveGen::knightMoves[64] = {0};
bool MoveGen::initialized = false;

U64 MoveGen::GenerateMoves(int square, const int offsets[], int numOffsets, std::function<bool(int, int)> isValidMove) {
    U64 attacks = 0ULL;
    int rank = square / 8;
    int file = square % 8;
    
    for (int i = 0; i < numOffsets; i++) {
        int targetSquare = square + offsets[i];
        
        if (targetSquare < 0 || targetSquare >= 64) continue;
        
        int targetRank = targetSquare / 8;
        int targetFile = targetSquare % 8;
        
        int rankDiff = abs(targetRank - rank);
        int fileDiff = abs(targetFile - file);
        
        if (isValidMove(rankDiff, fileDiff)) {
            attacks |= (1ULL << targetSquare);
        }
    }
    
    return attacks;
}

void MoveGen::InitKnightMoves() {
    int knightOffsets[8] = {17, 15, 10, 6, -6, -10, -15, -17};
    
    for (int square = 0; square < 64; square++) {
        knightMoves[square] = GenerateMoves(square, knightOffsets, 8,
            [](int rankDiff, int fileDiff) {
                return (rankDiff == 2 && fileDiff == 1) || (rankDiff == 1 && fileDiff == 2);
            });
    }
}

void MoveGen::InitKingMoves() {
    int kingOffsets[8] = {1, 9, 8, 7, -1, -9, -8, -7};
    
    for (int square = 0; square < 64; square++) {
        kingMoves[square] = GenerateMoves(square, kingOffsets, 8,
            [](int rankDiff, int fileDiff) {
                return rankDiff <= 1 && fileDiff <= 1;
            });
    }
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
