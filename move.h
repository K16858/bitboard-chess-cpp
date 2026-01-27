#ifndef MOVE_H
#define MOVE_H

#include "bitboard.h"

enum PieceType {
    NO_PIECE = 0,
    PAWN = 1,
    KNIGHT = 2,
    BISHOP = 3,
    ROOK = 4,
    QUEEN = 5,
    KING = 6
};

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

#endif

