#include "board.h"
#include "movegen.h"
#include "move.h"
#include <iostream>
#include <string>
#include <vector>

int main() {
    MoveGen::Init();
    
    Board board;
    board.Print();
    
    Board gameBoard;
    std::string input;
    while (true) {
        std::vector<Move> legalMoves;
        MoveGen::GenerateLegalMoves(gameBoard, legalMoves);
        gameBoard.Print();
        if (legalMoves.empty()) {
            if (gameBoard.IsInCheck(gameBoard.GetWhiteToMove())) {
                std::cout << "Checkmate. " << (gameBoard.GetWhiteToMove() ? "Black" : "White") << " wins.\n";
            } else {
                std::cout << "Stalemate. Draw.\n";
            }
            break;
        }
        std::cout << (gameBoard.GetWhiteToMove() ? "White" : "Black") << " to move (e.g. e2e4 or quit): ";
        std::getline(std::cin, input);
        if (input == "quit" || input == "q") break;
        if (input.size() >= 4) {
            Square from = StrToSquare(input.substr(0, 2));
            Square to = StrToSquare(input.substr(2, 2));
            const Move* found = nullptr;
            for (const Move& m : legalMoves)
                if (m.from == from && m.to == to) { found = &m; break; }
            if (found) gameBoard.MakeMove(*found);
            else std::cout << "Illegal move.\n";
        }
    }
    return 0;
}
