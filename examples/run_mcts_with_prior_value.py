#!/usr/bin/env python3
"""Example: run_mcts with optional prior and value callables."""

import sys
sys.path.insert(0, ".")

import chess_engine

chess_engine.init()
b = chess_engine.Board()

# prior(fen, uci_list) -> list[float]: same length as uci_list, weights per move
def uniform_prior(fen, uci_list):
    return [1.0] * len(uci_list)

# value(fen) -> float: root's view, in [-1, 1]
def const_value(fen):
    return 0.0

visits, root_value, root_visits = chess_engine.run_mcts(
    b, 100, 42, prior=uniform_prior, value=const_value
)
print("prior=uniform, value=0: root_visits=", root_visits, "root_value=", root_value)
print("visits len:", len(visits), "(matches legal_moves)")
print("ok")
