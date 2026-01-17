#include "bitboard.h"
#include <iostream>

Bitboard::Bitboard(U64 board) {
    this->board = board;
}

U64 Bitboard::GetBoard() const {
    return board;
}

void Bitboard::PrintBoard() const {
    std::cout << std::endl;
    for (int rank = 7; rank >= 0; rank--) {
        std::cout << rank + 1 << " ";
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            U64 bit = 1ULL << square;
            std::cout << (board & bit ? "1 " : ". ");
        }
        std::cout << std::endl;
    }
    std::cout << "  a b c d e f g h" << std::endl << std::endl;
}
