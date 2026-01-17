#include <iostream>
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

int main() {
    Bitboard bb(0x000000000000FF00);
    bb.PrintBoard();
    return 0;
}
