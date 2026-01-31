# ドキュメント

## ビルド詳細

### C++ 実行ファイル

- `make`: 実行ファイル `chess` を生成
- `make clean`: オブジェクトと実行ファイルを削除
- `make compile_commands`: clangd 用 `compile_commands.json` を生成

### Python 拡張

- `make deps`: pybind11 を `extern/` に取得（pip または curl）。初回のみ実行
- `make python`: `chess_engine.*.so` を生成。`python3-dev` と `make deps` 済みであること
- `make clean`: Python ビルド成果物も削除

## Python API

- `chess_engine.init()` — 1 回だけ呼ぶ（MoveGen 初期化）
- `chess_engine.Board(fen=None)` — 局面。`fen` 省略時は初期局面
- `board.set_fen(fen)` / `board.fen()` — FEN の設定・取得
- `board.legal_moves()` — 合法手の UCI 文字列リスト（順序固定）
- `board.push(uci)` / `board.pop()` — 1 手進める・戻す
- `board.result()` — 1=白勝ち, -1=黒勝ち, 0=引き分け, 2=進行中
- `board.white_to_move` — 手番（プロパティ）
- `chess_engine.run_mcts(board, iterations, seed, prior=None, value=None)` — MCTS 実行。戻り値 `(uci_list, visits, root_value, root_visits)`。`uci_list[i]` と `visits[i]` が対応（手の UCI と訪問数のペア）。
  - `prior`: callable なら `prior(fen, uci_list) -> list[float]`。fen は局面 FEN、uci_list は合法手 UCI リスト（`legal_moves()` と同じ順）。返り値の i 番目が i 番目の手の prior（正規化は C++ 側で行う）。
  - `value`: callable なら `value(fen) -> float`。fen は評価する局面の FEN。root 手番から見た値で [-1, 1] を返す想定。

## 例

- [../examples/test_binding.py](../examples/test_binding.py) — バインディングの最小テスト
- [../examples/run_mcts_with_prior_value.py](../examples/run_mcts_with_prior_value.py) — prior/value を渡す例
