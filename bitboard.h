#ifndef BITBOARD_H
#define BITBOARD_H

#include <cstdint>

typedef uint64_t U64;

enum Square {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
};

const U64 FILE_B = 0x0202020202020202ULL;
const U64 FILE_C = 0x0404040404040404ULL;
const U64 FILE_D = 0x0808080808080808ULL;
const U64 FILE_E = 0x1010101010101010ULL;
const U64 FILE_F = 0x2020202020202020ULL;
const U64 FILE_G = 0x4040404040404040ULL;

const U64 RANK_2 = 0x000000000000FF00ULL;
const U64 RANK_3 = 0x0000000000FF0000ULL;
const U64 RANK_4 = 0x00000000FF000000ULL;
const U64 RANK_5 = 0x000000FF00000000ULL;
const U64 RANK_6 = 0x0000FF0000000000ULL;
const U64 RANK_7 = 0x00FF000000000000ULL;

const U64 DIAGONAL_A1H8 = 0x8040201008040201ULL;
const U64 DIAGONAL_A8H1 = 0x0102040810204080ULL;

class Bitboard {
    public:
        Bitboard(U64 board);
        Bitboard() : board(0) {}
        U64 GetBoard() const;
        void SetBit(Square square);
        void ClearBit(Square square);
        bool GetBit(Square square) const;
        int CountBits() const;
        int GetLSB() const;
        int PopLSB();
        void PrintBoard() const;
    private:
        U64 board;
};

#endif
