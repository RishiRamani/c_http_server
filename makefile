CC = gcc
CFLAGS = -Wall -pthread
SRC = src/main.c src/server.c src/http.c src/handlers.c src/storage.c src/utils.c
OUT = server

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)