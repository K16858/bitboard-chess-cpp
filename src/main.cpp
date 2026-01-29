#include "board.hpp"
#include "movegen.hpp"
#include "move.hpp"
#include "mcts.hpp"
#include <iostream>
#include <string>

int main() {
    MoveGen::Init();

    // アンパッサンテスト: e2-e4, e7-e5, d2-d4 のあと黒が e5xd4 ep 可能
    // Board board;
    // board.MakeMove(Move(E2, E4, PAWN));
    // board.MakeMove(Move(E7, E5, PAWN));
    // board.MakeMove(Move(D2, D4, PAWN));
    // std::cout << "After e2-e4 e7-e5 d2-d4 (Black to move):" << std::endl;
    // board.Print();
    // std::vector<Move> moves;
    // MoveGen::GenerateLegalMoves(board, moves);
    // const Move* epMove = nullptr;
    // for (const Move& m : moves)
    //     if (m.from == E5 && m.to == D4 && m.pieceType == PAWN && m.capturedPiece == PAWN) {
    //         epMove = &m;
    //         break;
    //     }
    // std::cout << "En passant e5xd4 in legal moves? " << (epMove ? "YES" : "NO") << std::endl;
    // if (!epMove) {
    //     std::cout << "  Legal moves (from-to): ";
    //     for (const Move& m : moves) std::cout << SquareToStr(m.from) << SquareToStr(m.to) << " ";
    //     std::cout << std::endl;
    //     return 1;
    // }
    // U64 hBefore = board.GetZobristHash();
    // board.MakeMove(*epMove);
    // std::cout << "After black e5xd4 (en passant):" << std::endl;
    // board.Print();
    // board.UnmakeMove(*epMove);
    // U64 hAfter = board.GetZobristHash();
    // std::cout << "After Unmake: hash restored? " << (hBefore == hAfter ? "OK" : "FAIL") << std::endl;
    // std::cout << "En passant test done." << std::endl;

    Board board;
    board.Print();
    board.SetFromFen("b2r1r2/4nk2/p1q1p1p1/1p3pP1/3P1P2/P1N1B2R/1P2Q3/5RK1 w - - 0 31");
    board.Print();

    // MCTS
    std::mt19937 gen(std::random_device{}());
    // MCTSResult res = RunMCTS(board, 500, gen);
    // std::cout << "--- MCTS --- rootVisits=" << res.rootVisits << " rootValue=" << res.rootValue << "\n";
    // std::cout << "Top moves: ";
    // int top = static_cast<int>(res.visits.size()) < 5 ? static_cast<int>(res.visits.size()) : 5;
    // for (int i = 0; i < top; i++)
    //     std::cout << SquareToStr(res.visits[i].first.from) << SquareToStr(res.visits[i].first.to)
    //               << "(" << res.visits[i].second << ") ";
    // std::cout << "\n";

    // U64 h0 = board.GetZobristHash();
    // Move e2e4(E2, E4, PAWN);
    // board.MakeMove(e2e4);
    // U64 h1 = board.GetZobristHash();
    // board.UnmakeMove(e2e4);
    // U64 h2 = board.GetZobristHash();
    // std::cout << "Zobrist test: initial=" << h0 << " after e2e4=" << h1 << " after unmake=" << h2 << " (h0==h2? " << (h0 == h2 ? "OK" : "FAIL") << ")" << std::endl;

    // Board gameBoard;
    // std::string input;

    // int whiteWins = 0, blackWins = 0, draws = 0;
    // for (int i = 0; i < 100; i++) {
    //     GameResult r = MoveGen::DoRandomPlayout(board, gen);
    //     std::cout << "  Playout " << (i + 1) << ": " << (r == GameResult::WhiteWin ? "White win" : r == GameResult::BlackWin ? "Black win" : r == GameResult::Draw ? "Draw" : "Ongoing") << std::endl;
    //     if (r == GameResult::WhiteWin) whiteWins++;
    //     else if (r == GameResult::BlackWin) blackWins++;
    //     else if (r == GameResult::Draw) draws++;
    // }
    // std::cout << "  Summary: White=" << whiteWins << " Black=" << blackWins << " Draw=" << draws << std::endl;

    // MCTS AI
    Board gameBoard;
    std::string input;
    const int mctsIterations = 2000;
    std::cout << "--- Human (White) vs MCTS AI (Black). Input moves as e2e4, quit to exit ---\n";
    while (true) {
        std::vector<Move> legalMoves;
        MoveGen::GenerateLegalMoves(gameBoard, legalMoves);
        gameBoard.Print();
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
