# コンパイラとフラグの設定
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
DEBUGFLAGS = -g -O0

# ターゲット実行ファイル名
TARGET = chess

# ソースファイルとオブジェクトファイル
SRCS = main.cpp bitboard.cpp board.cpp movegen.cpp move.cpp
OBJS = $(SRCS:.cpp=.o)

# デフォルトターゲット
all: $(TARGET)

# 実行ファイルの作成
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# オブジェクトファイルの作成
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

.PHONY: all clean run debug rebuild

