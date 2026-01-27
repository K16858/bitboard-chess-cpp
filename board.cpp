#include "board.h"
#include "movegen.h"
#include <iostream>

Board::Board() {
    whitePawns.SetBoard(0x000000000000FF00ULL);
    whiteKnights.SetBoard(0x0000000000000042ULL);
    whiteBishops.SetBoard(0x0000000000000024ULL);
    whiteRooks.SetBoard(0x0000000000000081ULL);
    whiteQueens.SetBoard(0x0000000000000008ULL);
    whiteKings.SetBoard(0x0000000000000010ULL);
    
    blackPawns.SetBoard(0x00FF000000000000ULL);
    blackKnights.SetBoard(0x4200000000000000ULL);
    blackBishops.SetBoard(0x2400000000000000ULL);
    blackRooks.SetBoard(0x8100000000000000ULL);
    blackQueens.SetBoard(0x0800000000000000ULL);
    blackKings.SetBoard(0x1000000000000000ULL);
    
    Update();
}

void Board::Update() {
    allWhitePieces.SetBoard(
        whitePawns.GetBoard() | 
        whiteKnights.GetBoard() | 
        whiteBishops.GetBoard() | 
        whiteRooks.GetBoard() | 
        whiteQueens.GetBoard() | 
        whiteKings.GetBoard()
    );
    
    allBlackPieces.SetBoard(
        blackPawns.GetBoard() | 
        blackKnights.GetBoard() | 
        blackBishops.GetBoard() | 
        blackRooks.GetBoard() | 
        blackQueens.GetBoard() | 
        blackKings.GetBoard()
    );
    
    allPieces.SetBoard(
        allWhitePieces.GetBoard() | 
        allBlackPieces.GetBoard()
    );
}

void Board::Print() const {
    std::cout << std::endl;
    for (int rank = 7; rank >= 0; rank--) {
        std::cout << rank + 1 << " ";
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            char piece = '.';
            
            if (whitePawns.GetBit((Square)square)) piece = 'P';
            else if (whiteKnights.GetBit((Square)square)) piece = 'N';
            else if (whiteBishops.GetBit((Square)square)) piece = 'B';
            else if (whiteRooks.GetBit((Square)square)) piece = 'R';
            else if (whiteQueens.GetBit((Square)square)) piece = 'Q';
            else if (whiteKings.GetBit((Square)square)) piece = 'K';

            else if (blackPawns.GetBit((Square)square)) piece = 'p';
            else if (blackKnights.GetBit((Square)square)) piece = 'n';
            else if (blackBishops.GetBit((Square)square)) piece = 'b';
            else if (blackRooks.GetBit((Square)square)) piece = 'r';
            else if (blackQueens.GetBit((Square)square)) piece = 'q';
            else if (blackKings.GetBit((Square)square)) piece = 'k';
            
            std::cout << piece << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "  a b c d e f g h" << std::endl << std::endl;
}

int Board::GetPieceAt(Square square) const {
    if (whitePawns.GetBit(square)) return PAWN;
    if (whiteKnights.GetBit(square)) return KNIGHT;
    if (whiteBishops.GetBit(square)) return BISHOP;
    if (whiteRooks.GetBit(square)) return ROOK;
    if (whiteQueens.GetBit(square)) return QUEEN;
    if (whiteKings.GetBit(square)) return KING;
    if (blackPawns.GetBit(square)) return PAWN;
    if (blackKnights.GetBit(square)) return KNIGHT;
    if (blackBishops.GetBit(square)) return BISHOP;
    if (blackRooks.GetBit(square)) return ROOK;
    if (blackQueens.GetBit(square)) return QUEEN;
    if (blackKings.GetBit(square)) return KING;
    return NO_PIECE;
}

bool Board::IsSquareAttacked(Square square, bool byWhite) const {
    U64 sqBit = 1ULL << square;
    
    if (byWhite) {
        for (int sq = 0; sq < 64; sq++) {
            if (whitePawns.GetBit((Square)sq) && (MoveGen::GetPawnCaptures((Square)sq, true) & sqBit)) return true;
            if (whiteKnights.GetBit((Square)sq) && (MoveGen::GetKnightMoves((Square)sq) & sqBit)) return true;
            if (whiteBishops.GetBit((Square)sq) && (MoveGen::GetBishopMoves((Square)sq) & sqBit)) return true;
            if (whiteRooks.GetBit((Square)sq) && (MoveGen::GetRookMoves((Square)sq) & sqBit)) return true;
            if (whiteQueens.GetBit((Square)sq) && (MoveGen::GetQueenMoves((Square)sq) & sqBit)) return true;
            if (whiteKings.GetBit((Square)sq) && (MoveGen::GetKingMoves((Square)sq) & sqBit)) return true;
        }
    } else {
        for (int sq = 0; sq < 64; sq++) {
            if (blackPawns.GetBit((Square)sq) && (MoveGen::GetPawnCaptures((Square)sq, false) & sqBit)) return true;
            if (blackKnights.GetBit((Square)sq) && (MoveGen::GetKnightMoves((Square)sq) & sqBit)) return true;
            if (blackBishops.GetBit((Square)sq) && (MoveGen::GetBishopMoves((Square)sq) & sqBit)) return true;
            if (blackRooks.GetBit((Square)sq) && (MoveGen::GetRookMoves((Square)sq) & sqBit)) return true;
            if (blackQueens.GetBit((Square)sq) && (MoveGen::GetQueenMoves((Square)sq) & sqBit)) return true;
            if (blackKings.GetBit((Square)sq) && (MoveGen::GetKingMoves((Square)sq) & sqBit)) return true;
        }
    }
    return false;
}

bool Board::IsInCheck(bool white) const {
    const Bitboard& kings = white ? whiteKings : blackKings;
    int kingSq = kings.GetLSB();
    if (kingSq < 0) return false;
    return IsSquareAttacked((Square)kingSq, !white);
}

U64 Board::GetAllPieces() const {
    return allPieces.GetBoard();
}
