#include "move.hpp"
#include <cctype>

std::string SquareToStr(Square s) {
    int file = static_cast<int>(s) % 8;
    int rank = static_cast<int>(s) / 8;
    return std::string(1, 'a' + file) + std::string(1, '1' + rank);
}

Square StrToSquare(const std::string& s) {
    if (s.size() < 2) return A1;
    int file = std::tolower(static_cast<unsigned char>(s[0])) - 'a';
    int rank = s[1] - '1';
    if (file < 0 || file > 7 || rank < 0 || rank > 7) return A1;
    return static_cast<Square>(rank * 8 + file);
}
