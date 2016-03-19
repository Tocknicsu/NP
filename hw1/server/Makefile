CC = g++
FLAGS = -std=c++11 -g -Wunused-result
INCLUDE_DIR = ./include
SRC_DIR = ./src
BIN_DIR = ./bin
TEST_DIR = ./test
TMP_DIR = ./tmp

OBJS := function config log server sh

all:
	-mkdir bin
	-mkdir test
	-mkdir tmp
	#make clean
	make server
	cp ./bin/server ./server

$(OBJS): %: $(TEST_DIR)/%.test.cpp  $(foreach x,$(OBJS),$(TMP_DIR)/$x.o)
	$(CC) -o $(BIN_DIR)/$(*F) $< $(foreach x,$(OBJS),$(TMP_DIR)/$x.o) -I$(INCLUDE_DIR) $(FLAGS)
	#valgrind --leak-check=full --show-leak-kinds=all $(BIN_DIR)/$(*F)


$(foreach x,$(OBJS),$(TMP_DIR)/$x.o): $(TMP_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) -c $< -o $@ \
	-I$(INCLUDE_DIR) \
	$(FLAGS)


clean:
	-rm $(TMP_DIR)/*.o
