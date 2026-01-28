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
        bool whiteToMove;
        
        void ClearPieceAt(Square sq, int pieceType, bool white);
        void SetPieceAt(Square sq, int pieceType, bool white);
    public:
        Board();
        Board(const Board&) = default;
        Board& operator=(const Board&) = default;
        void Print() const;
        void Update();
        void MakeMove(const Move& move);
        void UnmakeMove(const Move& move);
        
        int GetPieceAt(Square square) const;
        bool IsSquareAttacked(Square square, bool byWhite) const;
        bool IsInCheck(bool white) const;
        U64 GetAllPieces() const;
        U64 GetWhitePieces() const;
        U64 GetBlackPieces() const;
        bool GetWhiteToMove() const { return whiteToMove; }
};

#endif