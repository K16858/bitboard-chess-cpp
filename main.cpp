#include "board.h"
#include "movegen.h"
#include "move.h"
#include <iostream>

int main() {
    MoveGen::Init();
    
    Board board;
    board.Print();
    
    std::cout << "=== Board extension test: ===" << std::endl;
    std::cout << "GetPieceAt(E1): " << board.GetPieceAt(E1) << " (KING=6)" << std::endl;
    std::cout << "IsSquareAttacked(E4, true): " << board.IsSquareAttacked(E4, true) << std::endl;
    std::cout << "IsInCheck(white): " << board.IsInCheck(true) << ", IsInCheck(black): " << board.IsInCheck(false) << std::endl;
    
    std::cout << "=== Rook E4 without occupancy: " << __builtin_popcountll(MoveGen::GetRookMoves(E4)) << " squares ===" << std::endl;
    std::cout << "=== Rook E4 with occupancy:  " << __builtin_popcountll(MoveGen::GetRookMoves(E4, board.GetAllPieces())) << " squares ===" << std::endl;
    
    std::cout << "=== White Pawn moves from E2: ===" << std::endl;
    Bitboard whitePawnMoves(MoveGen::GetPawnMoves(E2, true));
    whitePawnMoves.PrintBoard();
    
    std::cout << "=== White Pawn captures from E2: ===" << std::endl;
    Bitboard whitePawnCaptures(MoveGen::GetPawnCaptures(E2, true));
    whitePawnCaptures.PrintBoard();
    
    std::cout << "=== Black Pawn moves from E7: ===" << std::endl;
    Bitboard blackPawnMoves(MoveGen::GetPawnMoves(E7, false));
    blackPawnMoves.PrintBoard();
    
    std::cout << "=== Black Pawn captures from E7: ===" << std::endl;
    Bitboard blackPawnCaptures(MoveGen::GetPawnCaptures(E7, false));
    blackPawnCaptures.PrintBoard();
    
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
    
    std::cout << "=== Move structure test: ===" << std::endl;
    Move testMove(E2, E4, PAWN);
    std::cout << "Move from " << testMove.from << " to " << testMove.to 
              << ", piece: " << testMove.pieceType << std::endl;
    
    Move captureMove(E4, D5, PAWN, PAWN);
    std::cout << "Capture move from " << captureMove.from << " to " << captureMove.to 
              << ", captured: " << captureMove.capturedPiece << std::endl;
    
    return 0;
}
