# コンパイラとフラグの設定
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I include
DEBUGFLAGS = -g -O0

# ターゲット実行ファイル名
TARGET = chess

# ソースは src/、ヘッダは include/（.hpp）
VPATH = src
SRCS = main.cpp bitboard.cpp board.cpp movegen.cpp move.cpp zobrist.cpp mcts.cpp
OBJS = $(SRCS:.cpp=.o)

# デフォルトターゲット
all: $(TARGET)

# 実行ファイルの作成
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# オブジェクトファイルの作成（VPATH で src/ から .cpp を探す）
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# デバッグビルド
debug: CXXFLAGS = -std=c++17 -Wall -Wextra $(DEBUGFLAGS)
debug: clean $(TARGET)

# クリーンアップ
clean: clean-python
	rm -f $(OBJS) $(TARGET)

# 実行
run: $(TARGET)
	./$(TARGET)

# すべてクリーンして再ビルド
rebuild: clean all

# clangd 用 compile_commands.json を生成（エディタの警告対策）
compile_commands: compile_commands.json.in
	sed 's|@PROJECT_ROOT@|$(CURDIR)|g' $< > compile_commands.json

# Python 拡張モジュール（pybind11）。pip install -r requirements.txt で pybind11 を入れてから make python
PYTHON_SRCS = bitboard.cpp board.cpp movegen.cpp move.cpp zobrist.cpp mcts.cpp python_bindings.cpp
PYTHON_OBJS = $(addprefix build/python/,$(PYTHON_SRCS:.cpp=.o))
PYFLAGS = -fPIC $(shell python3 -m pybind11 --includes)
PYSUFFIX = $(shell python3-config --extension-suffix 2>/dev/null || echo .so)
PYMOD = chess_engine$(PYSUFFIX)

build/python/%.o: src/%.cpp
	@mkdir -p build/python
	$(CXX) $(CXXFLAGS) $(PYFLAGS) -c $< -o $@

python: $(PYMOD)

$(PYMOD): $(PYTHON_OBJS)
	$(CXX) -shared $(PYTHON_OBJS) -o $@ $(shell python3-config --ldflags 2>/dev/null || true)

clean: clean-python
clean-python:
	rm -rf build/python $(PYMOD)

.PHONY: all clean clean-python run debug rebuild compile_commands python

