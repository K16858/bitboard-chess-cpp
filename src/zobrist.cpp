#include "zobrist.hpp"
#include <random>

U64 Zobrist::table[64][12] = {{0}};
U64 Zobrist::sideKey = 0;
U64 Zobrist::castlingKeys[4] = {0};
U64 Zobrist::epKeys[16] = {0};
bool Zobrist::initialized = false;

void Zobrist::Init() {
    if (initialized) return;
    std::mt19937_64 gen(12345);
    for (int sq = 0; sq < 64; sq++)
        for (int i = 0; i < 12; i++)
            table[sq][i] = gen();
    sideKey = gen();
    for (int i = 0; i < 4; i++) castlingKeys[i] = gen();
    for (int i = 0; i < 16; i++) epKeys[i] = gen();
    initialized = true;
}

U64 Zobrist::GetPieceKey(Square sq, int pieceType, bool isWhite) {
    int idx = (pieceType - 1) + (isWhite ? 0 : 6);
    return table[static_cast<int>(sq)][idx];
}

U64 Zobrist::GetSideKey() {
    return sideKey;
}

U64 Zobrist::GetCastlingKey(int idx) {
    return castlingKeys[idx & 3];
}

U64 Zobrist::GetEnPassantKey(int epKeyIndex) {
    return epKeys[epKeyIndex & 15];
}
