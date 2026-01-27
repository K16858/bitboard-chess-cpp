#ifndef BOARD_H
#define BOARD_H

#include "bitboard.h"
#include "move.h"

class Board {
    private:
        Bitboard allWhitePieces;
        Bitboard allBlackPieces;
        Bitboard allPieces;

        Bitboard whitePawns;
        Bitboard whiteKnights;
        Bitboard whiteBishops;
        Bitboard whiteRooks;
        Bitboard whiteQueens;
        Bitboard whiteKings;
        Bitboard blackPawns;
        Bitboard blackKnights;
        Bitboard blackBishops;
        Bitboard blackRooks;
        Bitboard blackQueens;
        Bitboard blackKings;
    public:
        Board();
        void Print() const;
        void Update();
        
        int GetPieceAt(Square square) const;
        bool IsSquareAttacked(Square square, bool byWhite) const;
        bool IsInCheck(bool white) const;
};

#endif