#include "board.hpp"
#include "movegen.hpp"
#include "move.hpp"
#include "mcts.hpp"
#include <iostream>
#include <string>

int main() {
    MoveGen::Init();

    std::mt19937 gen(std::random_device{}());
    Board gameBoard;
    std::string input;
    const int mctsIterations = 2000;
    std::cout << "--- Human (White) vs MCTS AI (Black). Input moves as e2e4, quit to exit ---\n";
    while (true) {
        gameBoard.Print();
        std::vector<Move> legalMoves;
        MoveGen::GenerateLegalMoves(gameBoard, legalMoves);
        if (legalMoves.empty()) {
            if (gameBoard.IsInCheck(gameBoard.GetWhiteToMove()))
                std::cout << "Checkmate. " << (gameBoard.GetWhiteToMove() ? "Black" : "White") << " wins.\n";
            else
                std::cout << "Stalemate. Draw.\n";
            break;
        }
        if (gameBoard.GetWhiteToMove()) {
            std::cout << "White (you) to move (e.g. e2e4 or quit): ";
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
        } else {
            std::cout << "Black (MCTS AI) thinking... ";
            std::cout.flush();
            Move aiMove = GetBestMoveMCTS(gameBoard, mctsIterations, gen);
            std::cout << "plays " << SquareToStr(aiMove.from) << SquareToStr(aiMove.to) << "\n";
            gameBoard.MakeMove(aiMove);
        }
    }
    return 0;
}
