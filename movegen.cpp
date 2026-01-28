#include "movegen.h"
#include <algorithm>
#include <cstdlib>
#include <functional>

U64 MoveGen::whitePawnMoves[64] = {0};
U64 MoveGen::whitePawnCaptures[64] = {0};
U64 MoveGen::blackPawnMoves[64] = {0};
U64 MoveGen::blackPawnCaptures[64] = {0};
U64 MoveGen::rookMoves[64] = {0};
U64 MoveGen::bishopMoves[64] = {0};
U64 MoveGen::knightMoves[64] = {0};
U64 MoveGen::queenMoves[64] = {0};
U64 MoveGen::kingMoves[64] = {0};
bool MoveGen::initialized = false;

U64 MoveGen::GenerateMoves(int square, const int offsets[], int numOffsets, std::function<bool(int, int)> isValidMove) {
    U64 attacks = 0ULL;
    int rank = square / 8;
    int file = square % 8;
    
    for (int i = 0; i < numOffsets; i++) {
        int targetSquare = square + offsets[i];
        
        if (targetSquare < 0 || targetSquare >= 64) continue;
        
        int targetRank = targetSquare / 8;
        int targetFile = targetSquare % 8;
        
        int rankDiff = abs(targetRank - rank);
        int fileDiff = abs(targetFile - file);
        
        if (isValidMove(rankDiff, fileDiff)) {
            attacks |= (1ULL << targetSquare);
        }
    }
    
    return attacks;
}

U64 GenerateSlidingMoves(int square, const int directions[], int numDirections) {
    U64 attacks = 0ULL;
    int rank = square / 8;
    int file = square % 8;
    
    for (int i = 0; i < numDirections; i++) {
        int offset = directions[i];
        for (int distance = 1; distance < 8; distance++) {
            int targetSquare = square + offset * distance;
            
            if (targetSquare < 0 || targetSquare >= 64) break;
            
            int targetRank = targetSquare / 8;
            int targetFile = targetSquare % 8;
            
            int rankDiff = abs(targetRank - rank);
            int fileDiff = abs(targetFile - file);
            
            if (offset == 1 || offset == -1) {
                if (rankDiff != 0 || fileDiff != distance) break;
            } else if (offset == 8 || offset == -8) {
                if (fileDiff != 0 || rankDiff != distance) break;
            } else {
                if (rankDiff != distance || fileDiff != distance) break;
            }
            
            attacks |= (1ULL << targetSquare);
        }
    }
    
    return attacks;
}

static U64 GenerateSlidingMovesBlocked(int square, const int directions[], int numDirections, U64 occupancy) {
    U64 attacks = 0ULL;
    int rank = square / 8;
    int file = square % 8;
    
    for (int i = 0; i < numDirections; i++) {
        int offset = directions[i];
        for (int distance = 1; distance < 8; distance++) {
            int targetSquare = square + offset * distance;
            
            if (targetSquare < 0 || targetSquare >= 64) break;
            
            int targetRank = targetSquare / 8;
            int targetFile = targetSquare % 8;
            
            int rankDiff = abs(targetRank - rank);
            int fileDiff = abs(targetFile - file);
            
            if (offset == 1 || offset == -1) {
                if (rankDiff != 0 || fileDiff != distance) break;
            } else if (offset == 8 || offset == -8) {
                if (fileDiff != 0 || rankDiff != distance) break;
            } else {
                if (rankDiff != distance || fileDiff != distance) break;
            }
            
            attacks |= (1ULL << targetSquare);
            if (occupancy & (1ULL << targetSquare)) break;
        }
    }
    
    return attacks;
}

void MoveGen::InitKingMoves() {
    int kingOffsets[8] = {1, 9, 8, 7, -1, -9, -8, -7};
    
    for (int square = 0; square < 64; square++) {
        kingMoves[square] = GenerateMoves(square, kingOffsets, 8,
            [](int rankDiff, int fileDiff) {
                return rankDiff <= 1 && fileDiff <= 1;
            });
    }
}


void MoveGen::InitKnightMoves() {
    int knightOffsets[8] = {17, 15, 10, 6, -6, -10, -15, -17};
    
    for (int square = 0; square < 64; square++) {
        knightMoves[square] = GenerateMoves(square, knightOffsets, 8,
            [](int rankDiff, int fileDiff) {
                return (rankDiff == 2 && fileDiff == 1) || (rankDiff == 1 && fileDiff == 2);
            });
    }
}

void MoveGen::InitPawnMoves() {
    for (int square = 0; square < 64; square++) {
        int rank = square / 8;
        int file = square % 8;
        
        // White pawn moves
        U64 whiteMoves = 0ULL;
        if (rank < 7) {
            whiteMoves |= (1ULL << (square + 8));
            if (rank == 1) {
                whiteMoves |= (1ULL << (square + 16));
            }
        }
        whitePawnMoves[square] = whiteMoves;
        
        // White pawn captures
        U64 whiteCaptures = 0ULL;
        if (rank < 7) {
            if (file < 7) whiteCaptures |= (1ULL << (square + 9));
            if (file > 0) whiteCaptures |= (1ULL << (square + 7));
        }
        whitePawnCaptures[square] = whiteCaptures;
        
        // Black pawn moves
        U64 blackMoves = 0ULL;
        if (rank > 0) {
            blackMoves |= (1ULL << (square - 8));
            if (rank == 6) {
                blackMoves |= (1ULL << (square - 16));
            }
        }
        blackPawnMoves[square] = blackMoves;
        
        // Black pawn captures
        U64 blackCaptures = 0ULL;
        if (rank > 0) {
            if (file < 7) blackCaptures |= (1ULL << (square - 7));
            if (file > 0) blackCaptures |= (1ULL << (square - 9));
        }
        blackPawnCaptures[square] = blackCaptures;
    }
}

void MoveGen::InitRookMoves() {
    int rookDirections[4] = {1, -1, 8, -8};
    
    for (int square = 0; square < 64; square++) {
        rookMoves[square] = GenerateSlidingMoves(square, rookDirections, 4);
    }
}

void MoveGen::InitBishopMoves() {
    int bishopDirections[4] = {9, -9, 7, -7};
    
    for (int square = 0; square < 64; square++) {
        bishopMoves[square] = GenerateSlidingMoves(square, bishopDirections, 4);
    }
}

void MoveGen::InitQueenMoves() {
    int queenDirections[8] = {1, -1, 8, -8, 9, -9, 7, -7};
    
    for (int square = 0; square < 64; square++) {
        queenMoves[square] = GenerateSlidingMoves(square, queenDirections, 8);
    }
}

void MoveGen::Init() {
    if (!initialized) {
        InitPawnMoves();
        InitRookMoves();
        InitBishopMoves();
        InitKnightMoves();
        InitQueenMoves();
        InitKingMoves();
        initialized = true;
    }
}

U64 MoveGen::GetPawnMoves(Square square, bool isWhite) {
    return isWhite ? whitePawnMoves[square] : blackPawnMoves[square];
}

U64 MoveGen::GetPawnCaptures(Square square, bool isWhite) {
    return isWhite ? whitePawnCaptures[square] : blackPawnCaptures[square];
}

U64 MoveGen::GetRookMoves(Square square, U64 occupancy) {
    if (occupancy == 0) return rookMoves[square];
    int rookDirections[4] = {1, -1, 8, -8};
    return GenerateSlidingMovesBlocked(square, rookDirections, 4, occupancy);
}

U64 MoveGen::GetBishopMoves(Square square, U64 occupancy) {
    if (occupancy == 0) return bishopMoves[square];
    int bishopDirections[4] = {9, -9, 7, -7};
    return GenerateSlidingMovesBlocked(square, bishopDirections, 4, occupancy);
}

U64 MoveGen::GetKnightMoves(Square square) {
    return knightMoves[square];
}

U64 MoveGen::GetQueenMoves(Square square, U64 occupancy) {
    if (occupancy == 0) return queenMoves[square];
    int queenDirections[8] = {1, -1, 8, -8, 9, -9, 7, -7};
    return GenerateSlidingMovesBlocked(square, queenDirections, 8, occupancy);
}

U64 MoveGen::GetKingMoves(Square square) {
    return kingMoves[square];
}

void MoveGen::GenerateLegalMoves(Board& board, std::vector<Move>& moves) {
    moves.clear();
    bool wtm = board.GetWhiteToMove();
    U64 allPieces = board.GetAllPieces();
    U64 ownPieces = wtm ? board.GetWhitePieces() : board.GetBlackPieces();
    U64 oppPieces = wtm ? board.GetBlackPieces() : board.GetWhitePieces();
    int ep = board.GetEnPassantTarget();
    
    std::vector<Move> pseudoLegal;
    // キャスリング: キングが2マス動く手を pseudo-legal に追加（ルーク側・クイーン側、空きマス条件のみ）
    if (wtm) {
        if (board.CanWhiteKingsideCastle() && !(allPieces & ((1ULL << F1) | (1ULL << G1))))
            pseudoLegal.push_back(Move(E1, G1, KING));
        if (board.CanWhiteQueensideCastle() && !(allPieces & ((1ULL << B1) | (1ULL << C1) | (1ULL << D1))))
            pseudoLegal.push_back(Move(E1, C1, KING));
    } else {
        if (board.CanBlackKingsideCastle() && !(allPieces & ((1ULL << F8) | (1ULL << G8))))
            pseudoLegal.push_back(Move(E8, G8, KING));
        if (board.CanBlackQueensideCastle() && !(allPieces & ((1ULL << B8) | (1ULL << C8) | (1ULL << D8))))
            pseudoLegal.push_back(Move(E8, C8, KING));
    }
    for (int sq = 0; sq < 64; sq++) {
        if (!(ownPieces & (1ULL << sq))) continue;
        int pieceType = board.GetPieceAt((Square)sq);
        U64 targets = 0;
        switch (pieceType) {
            case PAWN: {
                targets = (GetPawnMoves((Square)sq, wtm) & ~allPieces)
                        | (GetPawnCaptures((Square)sq, wtm) & oppPieces);
                if (ep >= 0) {
                    int sr = sq / 8, sf = sq % 8, er = ep / 8, ef = ep % 8;
                    if (wtm && sr == 3 && er == 4 && (sf - ef) * (sf - ef) == 1)
                        targets |= (1ULL << ep);
                    if (!wtm && sr == 4 && er == 2 && (sf - ef) * (sf - ef) == 1)
                        targets |= (1ULL << ep);
                }
                break;
            }
            case KNIGHT:
                targets = GetKnightMoves((Square)sq) & ~ownPieces;
                break;
            case BISHOP:
                targets = GetBishopMoves((Square)sq, allPieces) & ~ownPieces;
                break;
            case ROOK:
                targets = GetRookMoves((Square)sq, allPieces) & ~ownPieces;
                break;
            case QUEEN:
                targets = GetQueenMoves((Square)sq, allPieces) & ~ownPieces;
                break;
            case KING:
                targets = GetKingMoves((Square)sq) & ~ownPieces;
                break;
            default:
                break;
        }
        for (int to = 0; to < 64; to++) {
            if (!(targets & (1ULL << to))) continue;
            int cp = NO_PIECE;
            if (pieceType == PAWN && ep >= 0 && to == ep)
                cp = PAWN;
            else if (oppPieces & (1ULL << to))
                cp = board.GetPieceAt((Square)to);
            bool promote = (pieceType == PAWN && ((wtm && to >= 56) || (!wtm && to < 8)));
            if (promote) {
                pseudoLegal.push_back(Move((Square)sq, (Square)to, PAWN, cp, QUEEN));
                pseudoLegal.push_back(Move((Square)sq, (Square)to, PAWN, cp, ROOK));
                pseudoLegal.push_back(Move((Square)sq, (Square)to, PAWN, cp, BISHOP));
                pseudoLegal.push_back(Move((Square)sq, (Square)to, PAWN, cp, KNIGHT));
            } else {
                pseudoLegal.push_back(Move((Square)sq, (Square)to, pieceType, cp));
            }
        }
    }
    for (const Move& m : pseudoLegal) {
        board.MakeMove(m);
        if (!board.IsInCheck(wtm))
            moves.push_back(m);
        board.UnmakeMove(m);
    }
    std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) {
        return a.from < b.from
            || (a.from == b.from && (a.to < b.to || (a.to == b.to && a.promotionPiece < b.promotionPiece)));
    });
}

GameResult MoveGen::GetGameResult(Board& board) {
    std::vector<Move> moves;
    GenerateLegalMoves(board, moves);
    if (!moves.empty()) return GameResult::Ongoing;
    bool wtm = board.GetWhiteToMove();
    if (board.IsInCheck(wtm))
        return wtm ? GameResult::BlackWin : GameResult::WhiteWin;
    return GameResult::Draw;
}

GameResult MoveGen::DoRandomPlayout(Board board, std::mt19937& gen) {
    while (true) {
        std::vector<Move> moves;
        GenerateLegalMoves(board, moves);
        if (moves.empty()) return GetGameResult(board);
        std::uniform_int_distribution<std::size_t> dist(0, moves.size() - 1);
        board.MakeMove(moves[dist(gen)]);
    }
}
