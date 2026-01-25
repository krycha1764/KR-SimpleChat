
CC=gcc
CFLAGS=-I. -Wall -O3 -c
LFLAGS=-I. -Wall -O3 -lpthread

.PHONY: all clean

all: server client

server: server.o TLV.o signals.o users.o daemon_init.o
	$(CC) $(LFLAGS) server.o TLV.o signals.o users.o daemon_init.o -o server

client: client.o TLV.o signals.o
	$(CC) $(LFLAGS) client.o TLV.o signals.o -o client

server.o: server.c
	$(CC) $(CFLAGS) server.c -o server.o

client.o: client.c
	$(CC) $(CFLAGS) client.c -o client.o

signals.o: signals.c signals.h
	$(CC) $(CFLAGS) signals.c -o signals.o

TLV.o: TLV.c TLV.h
	$(CC) $(CFLAGS) TLV.c -o TLV.o

users.o: users.c users.h
	$(CC) $(CFLAGS) users.c -o users.o

daemon_init.o: daemon_init.c daemon_init.h
	$(CC) $(CFLAGS) daemon_init.c -o daemon_init.o

clean:
	rm -rf server
	rm -rf client
	rm -rf *.o
