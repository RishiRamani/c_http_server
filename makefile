CC = gcc
CFLAGS = -Wall -Wextra

SRC = src/main.c src/server.c src/http.c src/utils.c
OUT = server

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)