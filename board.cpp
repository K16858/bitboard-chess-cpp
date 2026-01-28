#include "board.h"
#include "movegen.h"
#include "zobrist.h"
#include <iostream>

Board::Board() : whiteToMove(true), zobristHash(0), castlingRights_(0x0Fu), enPassantTarget_(-1), halfMoveClock_(0) {
    Zobrist::Init();
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
    ComputeZobristHash();
}

void Board::ComputeZobristHash() {
    zobristHash = 0;
    for (int sq = 0; sq < 64; sq++) {
        Square s = static_cast<Square>(sq);
        if (whitePawns.GetBit(s)) zobristHash ^= Zobrist::GetPieceKey(s, PAWN, true);
        else if (whiteKnights.GetBit(s)) zobristHash ^= Zobrist::GetPieceKey(s, KNIGHT, true);
        else if (whiteBishops.GetBit(s)) zobristHash ^= Zobrist::GetPieceKey(s, BISHOP, true);
        else if (whiteRooks.GetBit(s)) zobristHash ^= Zobrist::GetPieceKey(s, ROOK, true);
        else if (whiteQueens.GetBit(s)) zobristHash ^= Zobrist::GetPieceKey(s, QUEEN, true);
        else if (whiteKings.GetBit(s)) zobristHash ^= Zobrist::GetPieceKey(s, KING, true);
        else if (blackPawns.GetBit(s)) zobristHash ^= Zobrist::GetPieceKey(s, PAWN, false);
        else if (blackKnights.GetBit(s)) zobristHash ^= Zobrist::GetPieceKey(s, KNIGHT, false);
        else if (blackBishops.GetBit(s)) zobristHash ^= Zobrist::GetPieceKey(s, BISHOP, false);
        else if (blackRooks.GetBit(s)) zobristHash ^= Zobrist::GetPieceKey(s, ROOK, false);
        else if (blackQueens.GetBit(s)) zobristHash ^= Zobrist::GetPieceKey(s, QUEEN, false);
        else if (blackKings.GetBit(s)) zobristHash ^= Zobrist::GetPieceKey(s, KING, false);
    }
    if (!whiteToMove) zobristHash ^= Zobrist::GetSideKey();
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

U64 Board::GetWhitePieces() const {
    return allWhitePieces.GetBoard();
}

U64 Board::GetBlackPieces() const {
    return allBlackPieces.GetBoard();
}

void Board::ClearPieceAt(Square sq, int pieceType, bool white) {
    if (white) {
        if (pieceType == PAWN) whitePawns.ClearBit(sq);
        else if (pieceType == KNIGHT) whiteKnights.ClearBit(sq);
        else if (pieceType == BISHOP) whiteBishops.ClearBit(sq);
        else if (pieceType == ROOK) whiteRooks.ClearBit(sq);
        else if (pieceType == QUEEN) whiteQueens.ClearBit(sq);
        else if (pieceType == KING) whiteKings.ClearBit(sq);
    } else {
        if (pieceType == PAWN) blackPawns.ClearBit(sq);
        else if (pieceType == KNIGHT) blackKnights.ClearBit(sq);
        else if (pieceType == BISHOP) blackBishops.ClearBit(sq);
        else if (pieceType == ROOK) blackRooks.ClearBit(sq);
        else if (pieceType == QUEEN) blackQueens.ClearBit(sq);
        else if (pieceType == KING) blackKings.ClearBit(sq);
    }
}

void Board::SetPieceAt(Square sq, int pieceType, bool white) {
    if (white) {
        if (pieceType == PAWN) whitePawns.SetBit(sq);
        else if (pieceType == KNIGHT) whiteKnights.SetBit(sq);
        else if (pieceType == BISHOP) whiteBishops.SetBit(sq);
        else if (pieceType == ROOK) whiteRooks.SetBit(sq);
        else if (pieceType == QUEEN) whiteQueens.SetBit(sq);
        else if (pieceType == KING) whiteKings.SetBit(sq);
    } else {
        if (pieceType == PAWN) blackPawns.SetBit(sq);
        else if (pieceType == KNIGHT) blackKnights.SetBit(sq);
        else if (pieceType == BISHOP) blackBishops.SetBit(sq);
        else if (pieceType == ROOK) blackRooks.SetBit(sq);
        else if (pieceType == QUEEN) blackQueens.SetBit(sq);
        else if (pieceType == KING) blackKings.SetBit(sq);
    }
}

void Board::MakeMove(const Move& move) {
    bool wtm = whiteToMove;
    undoStack_.push_back({castlingRights_, enPassantTarget_, halfMoveClock_});

    bool enPassant = (move.pieceType == PAWN && move.capturedPiece == PAWN &&
                      GetPieceAt(move.to) == NO_PIECE);
    Square capSq = move.to;
    if (enPassant)
        capSq = wtm ? static_cast<Square>(static_cast<int>(move.to) - 8) : static_cast<Square>(static_cast<int>(move.to) + 8);

    zobristHash ^= Zobrist::GetPieceKey(move.from, move.pieceType, wtm);
    if (move.capturedPiece != NO_PIECE)
        zobristHash ^= Zobrist::GetPieceKey(capSq, move.capturedPiece, !wtm);
    int pieceToPlace = (move.promotionPiece != NO_PIECE) ? move.promotionPiece : move.pieceType;
    zobristHash ^= Zobrist::GetPieceKey(move.to, pieceToPlace, wtm);
    zobristHash ^= Zobrist::GetSideKey();
    if (move.capturedPiece != NO_PIECE) {
        ClearPieceAt(capSq, move.capturedPiece, !wtm);
    }
    ClearPieceAt(move.from, move.pieceType, wtm);
    SetPieceAt(move.to, pieceToPlace, wtm);
    whiteToMove = !whiteToMove;

    int fromRank = static_cast<int>(move.from) / 8, toRank = static_cast<int>(move.to) / 8;
    if (move.pieceType == PAWN && fromRank == 1 && toRank == 3)
        enPassantTarget_ = static_cast<int>(move.from) + 8;
    else if (move.pieceType == PAWN && fromRank == 6 && toRank == 4)
        enPassantTarget_ = static_cast<int>(move.from) - 8;
    else
        enPassantTarget_ = -1;

    if (move.pieceType == KING)
        castlingRights_ &= wtm ? static_cast<uint8_t>(~3u) : static_cast<uint8_t>(~12u);
    else if (move.pieceType == ROOK) {
        if (move.from == H1) castlingRights_ &= static_cast<uint8_t>(~1u);
        else if (move.from == A1) castlingRights_ &= static_cast<uint8_t>(~2u);
        else if (move.from == H8) castlingRights_ &= static_cast<uint8_t>(~4u);
        else if (move.from == A8) castlingRights_ &= static_cast<uint8_t>(~8u);
    }
    if (move.capturedPiece == ROOK) {
        if (move.to == H1) castlingRights_ &= static_cast<uint8_t>(~1u);
        else if (move.to == A1) castlingRights_ &= static_cast<uint8_t>(~2u);
        else if (move.to == H8) castlingRights_ &= static_cast<uint8_t>(~4u);
        else if (move.to == A8) castlingRights_ &= static_cast<uint8_t>(~8u);
    }

    if (move.capturedPiece != NO_PIECE || move.pieceType == PAWN)
        halfMoveClock_ = 0;
    else
        halfMoveClock_++;

    Update();
}

void Board::UnmakeMove(const Move& move) {
    whiteToMove = !whiteToMove;
    bool wtm = whiteToMove;
    bool enPassant = (move.pieceType == PAWN && move.capturedPiece == PAWN);
    Square capSq = move.to;
    if (enPassant)
        capSq = wtm ? static_cast<Square>(static_cast<int>(move.to) - 8) : static_cast<Square>(static_cast<int>(move.to) + 8);
    zobristHash ^= Zobrist::GetSideKey();
    int pieceToRemove = (move.promotionPiece != NO_PIECE) ? move.promotionPiece : move.pieceType;
    zobristHash ^= Zobrist::GetPieceKey(move.to, pieceToRemove, wtm);
    zobristHash ^= Zobrist::GetPieceKey(move.from, move.pieceType, wtm);
    if (move.capturedPiece != NO_PIECE)
        zobristHash ^= Zobrist::GetPieceKey(capSq, move.capturedPiece, !wtm);
    ClearPieceAt(move.to, pieceToRemove, wtm);
    SetPieceAt(move.from, move.pieceType, wtm);
    if (move.capturedPiece != NO_PIECE) {
        SetPieceAt(capSq, move.capturedPiece, !wtm);
    }
    BoardUndoState u = undoStack_.back();
    undoStack_.pop_back();
    castlingRights_ = u.castlingRights;
    enPassantTarget_ = u.enPassantTarget;
    halfMoveClock_ = u.halfMoveClock;
    Update();
}
