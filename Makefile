CC = gcc
SRC_DIR = ./src
OBJ_DIR = ./obj
OPTION = -Wall

all: main.o server.o server_helper.o db.o parson.o
	$(CC) -o main \
	$(OBJ_DIR)/main.o \
	$(OBJ_DIR)/server.o \
	$(OBJ_DIR)/server_helper.o \
	$(OBJ_DIR)/db.o \
	$(OBJ_DIR)/parser.o \
	-lsqlite3

main.o:
	$(CC) -o $(OBJ_DIR)/main.o -c $(SRC_DIR)/main.c $(OPTION)

server.o:
	$(CC) -o $(OBJ_DIR)/server.o -c $(SRC_DIR)/server.c $(OPTION)

server_helper.o:
	$(CC) -o $(OBJ_DIR)/server_helper.o -c $(SRC_DIR)/server_helper.c $(OPTION)

db.o:
	$(CC) -o $(OBJ_DIR)/db.o -c $(SRC_DIR)/db.c $(OPTION)

parson.o:
	$(CC) -o $(OBJ_DIR)/parser.o -c $(SRC_DIR)/parson/parson.c $(OPTION)
