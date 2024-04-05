CC = gcc
CFLAGS = -Wall

all: client serveur

client: client.c
	$(CC) $(CFLAGS) $^ -o $@

serveur: serveur.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f client serveur