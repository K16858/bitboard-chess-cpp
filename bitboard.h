#ifndef BITBOARD_H
#define BITBOARD_H

#include <cstdint>

typedef uint64_t U64;

class Bitboard {
    public:
        Bitboard(U64 board);
        U64 GetBoard() const;
        void PrintBoard() const;
    private:
        U64 board;
};

#endif
