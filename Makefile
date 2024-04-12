CC = gcc
CFLAGS = -Wall

all: client1 client2 serveur

client1: client1.c
	$(CC) $(CFLAGS) $^ -o $@

client2: client2.c
	$(CC) $(CFLAGS) $^ -o $@

serveur: serveur.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f client serveur