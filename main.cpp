#include "board.h"
#include "movegen.h"
#include <iostream>

int main() {
    MoveGen::Init();
    
    Board board;
    board.Print();
    
    std::cout << "Knight moves from E4:" << std::endl;
    Bitboard knightMoves(MoveGen::GetKnightMoves(E4));
    knightMoves.PrintBoard();
    
    std::cout << "Knight moves from B1:" << std::endl;
    Bitboard knightMovesB1(MoveGen::GetKnightMoves(B1));
    knightMovesB1.PrintBoard();
    
    return 0;
}
