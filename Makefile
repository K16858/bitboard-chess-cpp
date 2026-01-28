# コンパイラとフラグの設定
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I include
DEBUGFLAGS = -g -O0

# ターゲット実行ファイル名
TARGET = chess

# ソースは src/、ヘッダは include/（.hpp）
VPATH = src
SRCS = main.cpp bitboard.cpp board.cpp movegen.cpp move.cpp zobrist.cpp
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
clean:
	rm -f $(OBJS) $(TARGET)

# 実行
run: $(TARGET)
	./$(TARGET)

# すべてクリーンして再ビルド
rebuild: clean all

# clangd 用 compile_commands.json を生成（エディタの警告対策）
compile_commands: compile_commands.json.in
	sed 's|@PROJECT_ROOT@|$(CURDIR)|g' $< > compile_commands.json

.PHONY: all clean run debug rebuild compile_commands

