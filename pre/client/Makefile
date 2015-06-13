CC = g++ -g
FLAGS = -std=c++11 -g
INCLUDE_DIR = ./include
SRC_DIR = ./src
BIN_DIR = ./bin
TEST_DIR = ./test


all: $(SRC_DIR)/main.cpp config.o client.o  server.o
	$(CC) -o $(BIN_DIR)/judgecenter \
	$(SRC_DIR)/main.cpp \
	config.o client.o  server.o \
	-I$(INCLUDE_DIR) \
	$(FLAGS)

	valgrind --leak-check=full --show-leak-kinds=all $(BIN_DIR)/judgecenter

client: $(TEST_DIR)/client.test.cpp client.o config.o log.o function.o
	$(CC) -o $(BIN_DIR)/client.test \
	$(TEST_DIR)/client.test.cpp \
	client.o config.o log.o function.o \
	-I$(INCLUDE_DIR) \
	$(FLAGS)

	valgrind --leak-check=full --show-leak-kinds=all $(BIN_DIR)/client.test
	
server: $(TEST_DIR)/server.test.cpp server.o  config.o log.o function.o
	$(CC) -o $(BIN_DIR)/server.test \
	$(TEST_DIR)/server.test.cpp \
	server.o config.o log.o function.o \
	-I$(INCLUDE_DIR) \
	$(FLAGS)

	valgrind --leak-check=full --show-leak-kinds=all $(BIN_DIR)/server.test


config: $(TEST_DIR)/config.test.cpp config.o function.o
	$(CC) -o $(BIN_DIR)/config.test \
	$(TEST_DIR)/config.test.cpp \
	config.o function.o \
	-I$(INCLUDE_DIR) \
	$(FLAGS)

	valgrind --leak-check=full --show-leak-kinds=all $(BIN_DIR)/config.test

log: $(TEST_DIR)/log.test.cpp log.o
	$(CC) -o $(BIN_DIR)/log.test \
	$(TEST_DIR)/log.test.cpp \
	log.o \
	-I$(INCLUDE_DIR) \
	$(FLAGS)
	
	valgrind --leak-check=full --show-leak-kinds=all $(BIN_DIR)/log.test

function: $(TEST_DIR)/function.test.cpp function.o
	$(CC) -o $(BIN_DIR)/function.test \
	$(TEST_DIR)/function.test.cpp \
	function.o \
	-I$(INCLUDE_DIR) \
	$(FLAGS)
	
	valgrind --leak-check=full --show-leak-kinds=all $(BIN_DIR)/function.test

client.o: $(SRC_DIR)/client.cpp
	$(CC) -c $(SRC_DIR)/client.cpp \
	-I$(INCLUDE_DIR) \
	$(FLAGS)

server.o: $(SRC_DIR)/server.cpp
	$(CC) -c $(SRC_DIR)/server.cpp \
	-I$(INCLUDE_DIR) \
	$(FLAGS) 

config.o: $(SRC_DIR)/config.cpp
	$(CC) -c $(SRC_DIR)/config.cpp \
	-I$(INCLUDE_DIR) \
	$(FLAGS)

log.o: $(SRC_DIR)/log.cpp
	$(CC) -c $(SRC_DIR)/log.cpp \
	-I$(INCLUDE_DIR) \
	$(FLAGS)

function.o: $(SRC_DIR)/function.cpp
	$(CC) -c $(SRC_DIR)/function.cpp \
	-I$(INCLUDE_DIR) \
	$(FLAGS)

clean:
	rm *.o
