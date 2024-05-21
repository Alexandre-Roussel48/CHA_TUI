#ifndef CLIENT_CHAT
#define CLIENT_CHAT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>

typedef struct {
	int dS;
	int tailleMess;
    pthread_t tsaisie;
    pthread_t treception;
    pthread_t tfile;
} chat_args;

void createChat(chat_args* args, char* address, int port);
int recvMessage(chat_args *args);
int sendMessage(chat_args *args, char* message);
int askUsername(chat_args *args);
void* reception(void* t);
void* saisie(void* t);
int launchChat(chat_args* args);

#endif