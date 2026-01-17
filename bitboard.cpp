#include "bitboard.h"
#include <iostream>

Bitboard::Bitboard(U64 board) {
    this->board = board;
}

U64 Bitboard::GetBoard() const {
    return board;
}

void Bitboard::SetBit(Square square) {
    board |= 1ULL << square;
}

void Bitboard::ClearBit(Square square) {
    board &= ~(1ULL << square);
}

bool Bitboard::GetBit(Square square) const {
    return board & (1ULL << square);
}

int Bitboard::CountBits() const {
    return __builtin_popcountll(board);
}

int Bitboard::GetLSB() const {
    return __builtin_ctzll(board);
}

int Bitboard::PopLSB() {
    int lsb = GetLSB();
    board &= (board - 1);
    return lsb;
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
