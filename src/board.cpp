#include "board.hpp"
#include "movegen.hpp"
#include "zobrist.hpp"
#include <iostream>
#include <sstream>
#include <cctype>

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

namespace {
    static U64 CastlingEpHash(uint8_t castling, int epTarget) {
        U64 h = 0;
        if (castling & 1u) h ^= Zobrist::GetCastlingKey(0);
        if (castling & 2u) h ^= Zobrist::GetCastlingKey(1);
        if (castling & 4u) h ^= Zobrist::GetCastlingKey(2);
        if (castling & 8u) h ^= Zobrist::GetCastlingKey(3);
        if (epTarget >= 0) {
            int idx = (epTarget >= 16 && epTarget <= 23) ? (epTarget - 16)
                     : (epTarget >= 40 && epTarget <= 47) ? (epTarget - 32) : -1;
            if (idx >= 0) h ^= Zobrist::GetEnPassantKey(idx);
        }
        return h;
    }
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
    zobristHash ^= CastlingEpHash(castlingRights_, enPassantTarget_);
}

void Board::SetFromFen(const std::string& fen) {
    whitePawns.SetBoard(0); whiteKnights.SetBoard(0); whiteBishops.SetBoard(0);
    whiteRooks.SetBoard(0); whiteQueens.SetBoard(0); whiteKings.SetBoard(0);
    blackPawns.SetBoard(0); blackKnights.SetBoard(0); blackBishops.SetBoard(0);
    blackRooks.SetBoard(0); blackQueens.SetBoard(0); blackKings.SetBoard(0);
    std::istringstream iss(fen);
    std::string placement, active, castling, ep, halfStr;
    iss >> placement >> active >> castling >> ep >> halfStr;
    std::string ranks[8];
    size_t n = 0;
    for (size_t i = 0, j = 0; i <= placement.size() && n < 8; i++) {
        if (i == placement.size() || placement[i] == '/') {
            ranks[n++] = placement.substr(j, i - j);
            j = i + 1;
        }
    }
    for (int r = 0; r < 8 && r < static_cast<int>(n); r++) {
        int rankIdx = 7 - r;
        int file = 0;
        for (char c : ranks[r]) {
            if (file >= 8) break;
            if (std::isdigit(static_cast<unsigned char>(c))) {
                file += (c - '0');
                continue;
            }
            bool white = (std::isupper(static_cast<unsigned char>(c)) != 0);
            int pt = NO_PIECE;
            switch (std::tolower(static_cast<unsigned char>(c))) {
                case 'p': pt = PAWN; break;
                case 'n': pt = KNIGHT; break;
                case 'b': pt = BISHOP; break;
                case 'r': pt = ROOK; break;
                case 'q': pt = QUEEN; break;
                case 'k': pt = KING; break;
                default: break;
            }
            if (pt != NO_PIECE)
                SetPieceAt(static_cast<Square>(rankIdx * 8 + file), pt, white);
            file++;
        }
    }
    whiteToMove = (active.empty() || active[0] == 'w');
    castlingRights_ = 0;
    if (castling != "-") {
        for (char c : castling) {
            if (c == 'K') castlingRights_ |= 1u;
            else if (c == 'Q') castlingRights_ |= 2u;
            else if (c == 'k') castlingRights_ |= 4u;
            else if (c == 'q') castlingRights_ |= 8u;
        }
    }
    enPassantTarget_ = (ep == "-" || ep.empty()) ? -1 : static_cast<int>(StrToSquare(ep));
    halfMoveClock_ = 0;
    if (!halfStr.empty()) {
        int v = 0;
        for (char c : halfStr) { if (std::isdigit(static_cast<unsigned char>(c))) v = v * 10 + (c - '0'); else break; }
        halfMoveClock_ = v;
    }
    undoStack_.clear();
    Update();
    ComputeZobristHash();
}

std::string Board::GetFen() const {
    std::ostringstream oss;
    for (int r = 7; r >= 0; r--) {
        int empty = 0;
        for (int f = 0; f < 8; f++) {
            Square sq = static_cast<Square>(r * 8 + f);
            if (whitePawns.GetBit(sq)) { if (empty) { oss << empty; empty = 0; } oss << 'P'; }
            else if (whiteKnights.GetBit(sq)) { if (empty) { oss << empty; empty = 0; } oss << 'N'; }
            else if (whiteBishops.GetBit(sq)) { if (empty) { oss << empty; empty = 0; } oss << 'B'; }
            else if (whiteRooks.GetBit(sq)) { if (empty) { oss << empty; empty = 0; } oss << 'R'; }
            else if (whiteQueens.GetBit(sq)) { if (empty) { oss << empty; empty = 0; } oss << 'Q'; }
            else if (whiteKings.GetBit(sq)) { if (empty) { oss << empty; empty = 0; } oss << 'K'; }
            else if (blackPawns.GetBit(sq)) { if (empty) { oss << empty; empty = 0; } oss << 'p'; }
            else if (blackKnights.GetBit(sq)) { if (empty) { oss << empty; empty = 0; } oss << 'n'; }
            else if (blackBishops.GetBit(sq)) { if (empty) { oss << empty; empty = 0; } oss << 'b'; }
            else if (blackRooks.GetBit(sq)) { if (empty) { oss << empty; empty = 0; } oss << 'r'; }
            else if (blackQueens.GetBit(sq)) { if (empty) { oss << empty; empty = 0; } oss << 'q'; }
            else if (blackKings.GetBit(sq)) { if (empty) { oss << empty; empty = 0; } oss << 'k'; }
            else empty++;
        }
        if (empty) oss << empty;
        if (r > 0) oss << '/';
    }
    oss << (whiteToMove ? " w " : " b ");
    if (castlingRights_ == 0) oss << '-';
    else {
        if (castlingRights_ & 1u) oss << 'K';
        if (castlingRights_ & 2u) oss << 'Q';
        if (castlingRights_ & 4u) oss << 'k';
        if (castlingRights_ & 8u) oss << 'q';
    }
    oss << ' ';
    oss << (enPassantTarget_ >= 0 ? SquareToStr(static_cast<Square>(enPassantTarget_)) : "-");
    oss << ' ' << halfMoveClock_ << " 1";
    return oss.str();
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
    U64 occupancy = GetAllPieces();
    if (byWhite) {
        for (int sq = 0; sq < 64; sq++) {
            if (whitePawns.GetBit((Square)sq) && (MoveGen::GetPawnCaptures((Square)sq, true) & sqBit)) return true;
            if (whiteKnights.GetBit((Square)sq) && (MoveGen::GetKnightMoves((Square)sq) & sqBit)) return true;
            if (whiteBishops.GetBit((Square)sq) && (MoveGen::GetBishopMoves((Square)sq, occupancy) & sqBit)) return true;
            if (whiteRooks.GetBit((Square)sq) && (MoveGen::GetRookMoves((Square)sq, occupancy) & sqBit)) return true;
            if (whiteQueens.GetBit((Square)sq) && (MoveGen::GetQueenMoves((Square)sq, occupancy) & sqBit)) return true;
            if (whiteKings.GetBit((Square)sq) && (MoveGen::GetKingMoves((Square)sq) & sqBit)) return true;
        }
    } else {
        for (int sq = 0; sq < 64; sq++) {
            if (blackPawns.GetBit((Square)sq) && (MoveGen::GetPawnCaptures((Square)sq, false) & sqBit)) return true;
            if (blackKnights.GetBit((Square)sq) && (MoveGen::GetKnightMoves((Square)sq) & sqBit)) return true;
            if (blackBishops.GetBit((Square)sq) && (MoveGen::GetBishopMoves((Square)sq, occupancy) & sqBit)) return true;
            if (blackRooks.GetBit((Square)sq) && (MoveGen::GetRookMoves((Square)sq, occupancy) & sqBit)) return true;
            if (blackQueens.GetBit((Square)sq) && (MoveGen::GetQueenMoves((Square)sq, occupancy) & sqBit)) return true;
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

    zobristHash ^= CastlingEpHash(castlingRights_, enPassantTarget_);

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
    bool castling = (move.pieceType == KING && (move.from == E1 || move.from == E8) &&
                     (move.to == G1 || move.to == C1 || move.to == G8 || move.to == C8));
    if (castling) {
        Square rookFrom, rookTo;
        if (move.from == E1 && move.to == G1) { rookFrom = H1; rookTo = F1; }
        else if (move.from == E1 && move.to == C1) { rookFrom = A1; rookTo = D1; }
        else if (move.from == E8 && move.to == G8) { rookFrom = H8; rookTo = F8; }
        else { rookFrom = A8; rookTo = D8; }
        zobristHash ^= Zobrist::GetPieceKey(rookFrom, ROOK, wtm);
        zobristHash ^= Zobrist::GetPieceKey(rookTo, ROOK, wtm);
        ClearPieceAt(rookFrom, ROOK, wtm);
        SetPieceAt(rookTo, ROOK, wtm);
    }
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

    zobristHash ^= CastlingEpHash(castlingRights_, enPassantTarget_);

    if (move.capturedPiece != NO_PIECE || move.pieceType == PAWN)
        halfMoveClock_ = 0;
    else
        halfMoveClock_++;

    Update();
}

void Board::UnmakeMove(const Move& move) {
    zobristHash ^= CastlingEpHash(castlingRights_, enPassantTarget_);

    whiteToMove = !whiteToMove;
    // 反転後: whiteToMove == 戻した手を指した側（mover）。駒の復元は mover=wtm, 取った駒=!wtm
    bool wtm = whiteToMove;
    // En passant: pawn capture where the pawn landed on the ep rank (rank 3 or 6); otherwise capSq == move.to
    bool enPassant = (move.pieceType == PAWN && move.capturedPiece == PAWN &&
                      (static_cast<int>(move.to) / 8 == 2 || static_cast<int>(move.to) / 8 == 5));
    Square capSq = move.to;
    if (enPassant)
        capSq = wtm ? static_cast<Square>(static_cast<int>(move.to) - 8) : static_cast<Square>(static_cast<int>(move.to) + 8);
    bool castling = (move.pieceType == KING && (move.from == E1 || move.from == E8) &&
                     (move.to == G1 || move.to == C1 || move.to == G8 || move.to == C8));
    if (castling) {
        Square rookFrom, rookTo;
        if (move.from == E1 && move.to == G1) { rookFrom = H1; rookTo = F1; }
        else if (move.from == E1 && move.to == C1) { rookFrom = A1; rookTo = D1; }
        else if (move.from == E8 && move.to == G8) { rookFrom = H8; rookTo = F8; }
        else { rookFrom = A8; rookTo = D8; }
        ClearPieceAt(rookTo, ROOK, wtm);
        SetPieceAt(rookFrom, ROOK, wtm);
        zobristHash ^= Zobrist::GetPieceKey(rookTo, ROOK, wtm);
        zobristHash ^= Zobrist::GetPieceKey(rookFrom, ROOK, wtm);
    }
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
    zobristHash ^= CastlingEpHash(castlingRights_, enPassantTarget_);
    Update();
}
