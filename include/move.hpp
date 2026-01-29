#ifndef MOVE_HPP
#define MOVE_HPP

#include "bitboard.hpp"
#include <string>

enum PieceType {
    NO_PIECE = 0,
    PAWN = 1,
    KNIGHT = 2,
    BISHOP = 3,
    ROOK = 4,
    QUEEN = 5,
    KING = 6
};

enum class GameResult { WhiteWin = 1, BlackWin = -1, Draw = 0, Ongoing = 2 };

struct Move {
    Square from;
    Square to;
    int pieceType;
    int capturedPiece;
    int promotionPiece;
    
    Move() : from(A1), to(A1), pieceType(NO_PIECE), 
             capturedPiece(NO_PIECE), promotionPiece(NO_PIECE) {}
    
    Move(Square f, Square t, int pt, int cp = NO_PIECE, int pp = NO_PIECE)
        : from(f), to(t), pieceType(pt), capturedPiece(cp), promotionPiece(pp) {}
};

std::string SquareToStr(Square s);
Square StrToSquare(const std::string& s);

#endif

