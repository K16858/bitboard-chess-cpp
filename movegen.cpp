#include "movegen.h"
#include <cstdlib>
#include <functional>

U64 MoveGen::pawnMoves[64] = {0};
U64 MoveGen::rookMoves[64] = {0};
U64 MoveGen::bishopMoves[64] = {0};
U64 MoveGen::knightMoves[64] = {0};
U64 MoveGen::queenMoves[64] = {0};
U64 MoveGen::kingMoves[64] = {0};
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

U64 GenerateSlidingMoves(int square, const int directions[], int numDirections) {
    U64 attacks = 0ULL;
    int rank = square / 8;
    int file = square % 8;
    
    for (int i = 0; i < numDirections; i++) {
        int offset = directions[i];
        for (int distance = 1; distance < 8; distance++) {
            int targetSquare = square + offset * distance;
            
            if (targetSquare < 0 || targetSquare >= 64) break;
            
            int targetRank = targetSquare / 8;
            int targetFile = targetSquare % 8;
            
            int rankDiff = abs(targetRank - rank);
            int fileDiff = abs(targetFile - file);
            
            if (offset == 1 || offset == -1) {
                if (rankDiff != 0 || fileDiff != distance) break;
            } else if (offset == 8 || offset == -8) {
                if (fileDiff != 0 || rankDiff != distance) break;
            } else {
                if (rankDiff != distance || fileDiff != distance) break;
            }
            
            attacks |= (1ULL << targetSquare);
        }
    }
    
    return attacks;
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


void MoveGen::InitKnightMoves() {
    int knightOffsets[8] = {17, 15, 10, 6, -6, -10, -15, -17};
    
    for (int square = 0; square < 64; square++) {
        knightMoves[square] = GenerateMoves(square, knightOffsets, 8,
            [](int rankDiff, int fileDiff) {
                return (rankDiff == 2 && fileDiff == 1) || (rankDiff == 1 && fileDiff == 2);
            });
    }
}

void MoveGen::InitPawnMoves() {
    for (int square = 0; square < 64; square++) {
        int rank = square / 8;
        U64 moves = 0ULL;
        if (rank < 7) moves |= (1ULL << (square + 8));
        if (rank > 0) moves |= (1ULL << (square - 8));
        pawnMoves[square] = moves;
    }
}

void MoveGen::InitRookMoves() {
    int rookDirections[4] = {1, -1, 8, -8};
    
    for (int square = 0; square < 64; square++) {
        rookMoves[square] = GenerateSlidingMoves(square, rookDirections, 4);
    }
}

void MoveGen::InitBishopMoves() {
    int bishopDirections[4] = {9, -9, 7, -7};
    
    for (int square = 0; square < 64; square++) {
        bishopMoves[square] = GenerateSlidingMoves(square, bishopDirections, 4);
    }
}

void MoveGen::InitQueenMoves() {
    int queenDirections[8] = {1, -1, 8, -8, 9, -9, 7, -7};
    
    for (int square = 0; square < 64; square++) {
        queenMoves[square] = GenerateSlidingMoves(square, queenDirections, 8);
    }
}

void MoveGen::Init() {
    if (!initialized) {
        InitPawnMoves();
        InitRookMoves();
        InitBishopMoves();
        InitKnightMoves();
        InitQueenMoves();
        InitKingMoves();
        initialized = true;
    }
}

U64 MoveGen::GetPawnMoves(Square square) {
    return pawnMoves[square];
}

U64 MoveGen::GetRookMoves(Square square) {
    return rookMoves[square];
}

U64 MoveGen::GetBishopMoves(Square square) {
    return bishopMoves[square];
}

U64 MoveGen::GetKnightMoves(Square square) {
    return knightMoves[square];
}

U64 MoveGen::GetQueenMoves(Square square) {
    return queenMoves[square];
}

U64 MoveGen::GetKingMoves(Square square) {
    return kingMoves[square];
}
