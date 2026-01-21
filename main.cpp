#include "board.h"
#include "movegen.h"
#include <iostream>

int main() {
    MoveGen::Init();
    
    Board board;
    board.Print();
    
    std::cout << "=== Pawn moves from E2: ===" << std::endl;
    Bitboard pawnMoves(MoveGen::GetPawnMoves(E2));
    pawnMoves.PrintBoard();
    
    std::cout << "=== Knight moves from E4: ===" << std::endl;
    Bitboard knightMoves(MoveGen::GetKnightMoves(E4));
    knightMoves.PrintBoard();
    
    std::cout << "=== Bishop moves from E4: ===" << std::endl;
    Bitboard bishopMoves(MoveGen::GetBishopMoves(E4));
    bishopMoves.PrintBoard();
    
    std::cout << "=== Rook moves from E4: ===" << std::endl;
    Bitboard rookMoves(MoveGen::GetRookMoves(E4));
    rookMoves.PrintBoard();
    
    std::cout << "=== Queen moves from E4: ===" << std::endl;
    Bitboard queenMoves(MoveGen::GetQueenMoves(E4));
    queenMoves.PrintBoard();
    
    std::cout << "=== King moves from E4: ===" << std::endl;
    Bitboard kingMoves(MoveGen::GetKingMoves(E4));
    kingMoves.PrintBoard();
    
    std::cout << "=== King moves from A1: ===" << std::endl;
    Bitboard kingMovesCorner(MoveGen::GetKingMoves(A1));
    kingMovesCorner.PrintBoard();
    
    return 0;
}
