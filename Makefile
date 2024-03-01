CC = gcc
SRC_DIR = ./src
OBJ_DIR = ./obj
HANDLERS_DIR = $(SRC_DIR)/handlers
OPTION = -Wall

all: main.o server.o server_helper.o db.o parson.o root.o not_found.o rooms.o
	$(CC) -o main \
	$(OBJ_DIR)/server_helper.o \
	$(OBJ_DIR)/db.o \
	$(OBJ_DIR)/parson.o \
	$(OBJ_DIR)/root.o \
	$(OBJ_DIR)/not_found.o \
	$(OBJ_DIR)/rooms.o \
	$(OBJ_DIR)/server.o \
	$(OBJ_DIR)/main.o \
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
	$(CC) -o $(OBJ_DIR)/parson.o -c $(SRC_DIR)/parson/parson.c $(OPTION)

root.o:
	$(CC) -o $(OBJ_DIR)/root.o -c $(HANDLERS_DIR)/root.c $(OPTION)

not_found.o:
	$(CC) -o $(OBJ_DIR)/not_found.o -c $(HANDLERS_DIR)/404/not_found.c $(OPTION)

rooms.o:
	$(CC) -o $(OBJ_DIR)/rooms.o -c $(HANDLERS_DIR)/rooms/rooms.c $(OPTION)