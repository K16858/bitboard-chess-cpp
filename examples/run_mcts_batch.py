#!/usr/bin/env python3
"""Example: run_mcts with batch_prior / batch_value (fewer Python↔C++ calls)."""

import sys
sys.path.insert(0, ".")

import chess_engine

chess_engine.init()
b = chess_engine.Board()

# 呼び出し回数を数えて、バッチで減っていることを確認
prior_calls = []
value_calls = []

# batch_prior(fen_list, uci_list_per_fen) -> list[list[float]]
def batch_prior(fen_list, uci_list_per_fen):
    prior_calls.append(len(fen_list))
    return [[1.0] * len(uci) for uci in uci_list_per_fen]

# batch_value(fen_list) -> list[float]
def batch_value(fen_list):
    value_calls.append(len(fen_list))
    return [0.0] * len(fen_list)

uci_list, visits, root_value, root_visits = chess_engine.run_mcts(
    b, 800, 42,
    batch_prior=batch_prior,
    batch_value=batch_value,
    batch_size=32,
)

print("iterations=800, batch_size=32")
print("root_visits=", root_visits, "root_value=", root_value)
print("batch_prior calls: %d (total positions: %d)" % (len(prior_calls), sum(prior_calls)))
print("batch_value calls: %d (total positions: %d)" % (len(value_calls), sum(value_calls)))
print("→ Python↔C++ round-trips: %d prior + %d value = %d (vs ~1600 with single-call)" % (
    len(prior_calls), len(value_calls), len(prior_calls) + len(value_calls)))
print("e.g. best moves:", list(zip(uci_list[:5], visits[:5])))
print("ok")
