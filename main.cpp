#include <iostream>
#include <cstdint>

typedef uint64_t U64;

class Bitboard {
    public:
        Bitboard(U64 board);
        U64 GetBoard() const;
    private:
        U64 board;
};

Bitboard::Bitboard(U64 board) {
    this->board = board;
}

U64 Bitboard::GetBoard() const {
    return board;
}

int main() {
    Bitboard bb(0x0000000000000000);
    std::cout << bb.GetBoard() << std::endl;
    return 0;
}
