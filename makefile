
CC=gcc
CFLAGS=-I. -Wall -O3
LFLAGS=

.PHONY: all clean

all: server client

server: server.c
	$(CC) $(CFLAGS) server.c -o server

client: client.c
	$(CC) $(CFLAGS) client.c -o client

clean:
	rm -rf server
	rm -rf client
	rm -rf *.o
