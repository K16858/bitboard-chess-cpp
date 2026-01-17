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

const U64 FILE_A = 0x0101010101010101;
const U64 FILE_H = 0x8080808080808080;
const U64 RANK_1 = 0x00000000000000FF;
const U64 RANK_8 = 0xFF00000000000000;

class Bitboard {
    public:
        Bitboard(U64 board);
        U64 GetBoard() const;
        void PrintBoard() const;
    private:
        U64 board;
};

#endif
