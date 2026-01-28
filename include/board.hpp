#ifndef BOARD_H
#define BOARD_H

#include "bitboard.hpp"
#include "move.hpp"
#include <cstdint>
#include <string>
#include <vector>

struct BoardUndoState {
    uint8_t castlingRights;
    int enPassantTarget;
    int halfMoveClock;
};

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
        U64 zobristHash;
        uint8_t castlingRights_;  // bit0=白キング側, bit1=白クイーン側, bit2=黒キング側, bit3=黒クイーン側. 1=可能
        int enPassantTarget_;     // アンパッサン可能な「取られるマス」の square index (0-63), なければ -1
        int halfMoveClock_;       // 50手ルール用。キャプチャ/ポーン移動で0に、それ以外で+1
        std::vector<BoardUndoState> undoStack_;

        void ClearPieceAt(Square sq, int pieceType, bool white);
        void SetPieceAt(Square sq, int pieceType, bool white);
        void ComputeZobristHash();
    public:
        Board();
        Board(const Board&) = default;
        Board& operator=(const Board&) = default;
        void Print() const;
        void Update();
        void SetFromFen(const std::string& fen);
        void MakeMove(const Move& move);
        void UnmakeMove(const Move& move);
        
        int GetPieceAt(Square square) const;
        bool IsSquareAttacked(Square square, bool byWhite) const;
        bool IsInCheck(bool white) const;
        U64 GetAllPieces() const;
        U64 GetWhitePieces() const;
        U64 GetBlackPieces() const;
        bool GetWhiteToMove() const { return whiteToMove; }
        U64 GetZobristHash() const { return zobristHash; }
        // 特殊ルール用 Getter（現状は初期値のまま。MakeMove/UnmakeMove での更新は後続コミット）
        bool CanWhiteKingsideCastle() const { return (castlingRights_ & 1u) != 0; }
        bool CanWhiteQueensideCastle() const { return (castlingRights_ & 2u) != 0; }
        bool CanBlackKingsideCastle() const { return (castlingRights_ & 4u) != 0; }
        bool CanBlackQueensideCastle() const { return (castlingRights_ & 8u) != 0; }
        int GetEnPassantTarget() const { return enPassantTarget_; }
        int GetHalfMoveClock() const { return halfMoveClock_; }
};

#endif