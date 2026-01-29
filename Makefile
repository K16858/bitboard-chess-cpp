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

# pybind11: extern/ にあればそれを使う（make deps で取得）。Python.h 用に python3-config --includes も必要。
PYBIND11_INCLUDES := $(if $(wildcard extern/pybind11/include),-I extern/pybind11/include $(shell python3-config --includes 2>/dev/null),$(shell python3 -m pybind11 --includes 2>/dev/null))
PYTHON_INCLUDES := $(shell python3-config --includes 2>/dev/null)

# clangd 用 compile_commands.json を生成。make deps 後・python3-dev 入れてから実行。
compile_commands: compile_commands.json.in
	sed -e 's|@PROJECT_ROOT@|$(CURDIR)|g' -e 's|@PYBIND11_INCLUDES@|$(PYBIND11_INCLUDES)|g' -e 's|@PYTHON_INCLUDES@|$(PYTHON_INCLUDES)|g' $< > compile_commands.json

# Python 拡張モジュール（pybind11）。make deps で extern に取得するか pip install -r requirements.txt
PYTHON_SRCS = bitboard.cpp board.cpp movegen.cpp move.cpp zobrist.cpp mcts.cpp python_bindings.cpp
PYTHON_OBJS = $(addprefix build/python/,$(PYTHON_SRCS:.cpp=.o))
PYFLAGS = -fPIC $(PYBIND11_INCLUDES)
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

# pybind11 を extern/ に取得（IDE と make python の両方で include 解決）。pip が無い場合は curl で取得。
deps:
	@mkdir -p extern
	@if [ -d extern/pybind11/include ]; then echo "pybind11 already in extern/"; exit 0; fi; \
	if python3 -m pip install --target extern pybind11 2>/dev/null; then echo "pybind11 installed via pip"; exit 0; fi; \
	curl -sL https://github.com/pybind/pybind11/archive/refs/tags/v2.11.1.tar.gz | tar xz -C extern && mv extern/pybind11-2.11.1 extern/pybind11 && echo "pybind11 fetched via curl"

.PHONY: all clean clean-python run debug rebuild compile_commands python deps

