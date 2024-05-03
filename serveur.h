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

typedef struct {
    int errorCode;
    char* message;
} MessageResult;

void init();
void connect_users();
int empty_client();
void *transmission(void *t);
void broadcast(int index, char* msg, int messageLength);