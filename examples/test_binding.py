#!/usr/bin/env python3
"""Minimal test for chess_engine Python binding. Run from repo root after: make python"""

import sys
sys.path.insert(0, ".")

import chess_engine

chess_engine.init()
b = chess_engine.Board()

print("legal_moves:", len(b.legal_moves()), "e.g.", b.legal_moves()[:5])
print("white_to_move:", b.white_to_move, "result:", b.result())

b.push("e2e4")
print("after e2e4: legal_moves:", len(b.legal_moves()))

uci_list, visits, root_value, root_visits = chess_engine.run_mcts(b, 300, 42)
print("run_mcts(300, 42): root_visits=", root_visits, "root_value=", round(root_value, 3))
print("uci_list[i] and visits[i] paired:", len(uci_list) == len(visits), "e.g.", list(zip(uci_list[:3], visits[:3])))

b.pop()
print("after pop: legal_moves:", len(b.legal_moves()))
print("ok")
