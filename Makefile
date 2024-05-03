CC = gcc
CFLAGS = -Wall -o
SRCSSERV = srcServeur/*.c

all: client serveur

client: client.c
	$(CC) -Wall $^ -o $@

serveur: serveur.c
	$(CC) $(CFLAGS) $@ $^ $(SRCSSERV)

clean:
	rm -f client serveur