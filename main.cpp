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

    std::mt19937 gen(std::random_device{}());
    int whiteWins = 0, blackWins = 0, draws = 0;
    for (int i = 0; i < 5; i++) {
        GameResult r = MoveGen::DoRandomPlayout(board, gen);
        std::cout << "  Playout " << (i + 1) << ": " << (r == GameResult::WhiteWin ? "White win" : r == GameResult::BlackWin ? "Black win" : r == GameResult::Draw ? "Draw" : "Ongoing") << std::endl;
        if (r == GameResult::WhiteWin) whiteWins++;
        else if (r == GameResult::BlackWin) blackWins++;
        else if (r == GameResult::Draw) draws++;
    }
    std::cout << "  Summary: White=" << whiteWins << " Black=" << blackWins << " Draw=" << draws << std::endl;

    // while (true) {
    //     std::vector<Move> legalMoves;
    //     MoveGen::GenerateLegalMoves(gameBoard, legalMoves);
    //     gameBoard.Print();
    //     if (legalMoves.empty()) {
    //         if (gameBoard.IsInCheck(gameBoard.GetWhiteToMove())) {
    //             std::cout << "Checkmate. " << (gameBoard.GetWhiteToMove() ? "Black" : "White") << " wins.\n";
    //         } else {
    //             std::cout << "Stalemate. Draw.\n";
    //         }
    //         break;
    //     }
    //     std::cout << (gameBoard.GetWhiteToMove() ? "White" : "Black") << " to move (e.g. e2e4 or quit): ";
    //     std::getline(std::cin, input);
    //     if (input == "quit" || input == "q") break;
    //     if (input.size() >= 4) {
    //         Square from = StrToSquare(input.substr(0, 2));
    //         Square to = StrToSquare(input.substr(2, 2));
    //         const Move* found = nullptr;
    //         for (const Move& m : legalMoves)
    //             if (m.from == from && m.to == to) { found = &m; break; }
    //         if (found) gameBoard.MakeMove(*found);
    //         else std::cout << "Illegal move.\n";
    //     }
    // }
    return 0;
}
