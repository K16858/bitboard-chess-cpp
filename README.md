# bitboard-chess-cpp

Bitboardベースのチェスエンジン。MCTSとPythonバインディング。

## ビルド

```bash
make          # 実行ファイル chess
make python   # Python 拡張 chess_engine.*.so（事前に make deps）
make deps     # pybind11 を extern/ に取得（初回のみ）
```

## 使い方

- **C++**: `./chess` で対局デモ
- **Python**: `make python` 後、`import chess_engine` → `chess_engine.init()` → `Board()` / `run_mcts()` / `fen()` など

```python
import chess_engine
chess_engine.init()
b = chess_engine.Board()
uci_list, visits, value, n = chess_engine.run_mcts(b, 500, 42)  # uci_list[i] と visits[i] が対応
```

詳細は [docs/](docs/) を参照。
