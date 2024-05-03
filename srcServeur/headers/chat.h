#ifndef SERVEUR_CHAT
#define SERVEUR_CHAT

#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
	int ad;
	char* username;
	pthread_t thread;
} user;

typedef struct {
	int dS;
	int nb_clients;
	pthread_mutex_t mutex_lock;
	user* users;
} chat_args;

void createChat(int nb_clients, int port, chat_args* args);
int acceptUser(chat_args* args);
void launchChat(int index, chat_args* args);
void* transmission(void *args);
int recvMsgLength(int index, user* users);
int recvMsg(int index, int msgLength, char** msg, user* users);
int sendMsg(int index, char* msg, int msgLength, user* users);
void broadcast(int index, char* msg, int msgLength, chat_args* args);

#endif