#include "board.h"
#include <iostream>

Board::Board() {
    whitePawns.SetBoard(0x000000000000FF00ULL);
    whiteKnights.SetBoard(0x0000000000000042ULL);
    whiteBishops.SetBoard(0x0000000000000024ULL);
    whiteRooks.SetBoard(0x0000000000000081ULL);
    whiteQueens.SetBoard(0x0000000000000008ULL);
    whiteKings.SetBoard(0x0000000000000010ULL);
    
    blackPawns.SetBoard(0x00FF000000000000ULL);
    blackKnights.SetBoard(0x4200000000000000ULL);
    blackBishops.SetBoard(0x2400000000000000ULL);
    blackRooks.SetBoard(0x8100000000000000ULL);
    blackQueens.SetBoard(0x0800000000000000ULL);
    blackKings.SetBoard(0x1000000000000000ULL);
    
    Update();
}

void Board::Update() {
    allWhitePieces.SetBoard(
        whitePawns.GetBoard() | 
        whiteKnights.GetBoard() | 
        whiteBishops.GetBoard() | 
        whiteRooks.GetBoard() | 
        whiteQueens.GetBoard() | 
        whiteKings.GetBoard()
    );
    
    allBlackPieces.SetBoard(
        blackPawns.GetBoard() | 
        blackKnights.GetBoard() | 
        blackBishops.GetBoard() | 
        blackRooks.GetBoard() | 
        blackQueens.GetBoard() | 
        blackKings.GetBoard()
    );
    
    allPieces.SetBoard(
        allWhitePieces.GetBoard() | 
        allBlackPieces.GetBoard()
    );
}

void Board::Print() const {
    std::cout << std::endl;
    for (int rank = 7; rank >= 0; rank--) {
        std::cout << rank + 1 << " ";
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            char piece = '.';
            
            if (whitePawns.GetBit((Square)square)) piece = 'P';
            else if (whiteKnights.GetBit((Square)square)) piece = 'N';
            else if (whiteBishops.GetBit((Square)square)) piece = 'B';
            else if (whiteRooks.GetBit((Square)square)) piece = 'R';
            else if (whiteQueens.GetBit((Square)square)) piece = 'Q';
            else if (whiteKings.GetBit((Square)square)) piece = 'K';

            else if (blackPawns.GetBit((Square)square)) piece = 'p';
            else if (blackKnights.GetBit((Square)square)) piece = 'n';
            else if (blackBishops.GetBit((Square)square)) piece = 'b';
            else if (blackRooks.GetBit((Square)square)) piece = 'r';
            else if (blackQueens.GetBit((Square)square)) piece = 'q';
            else if (blackKings.GetBit((Square)square)) piece = 'k';
            
            std::cout << piece << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "  a b c d e f g h" << std::endl << std::endl;
}
