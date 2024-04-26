#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

struct User {
	int ad;
	char* username;
	pthread_t thread;
};

typedef struct User User;

void init();
void connect_users();
int empty_client();
void *transmission(void *t);