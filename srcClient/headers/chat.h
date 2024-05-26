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

/* chat.c */
int countLines(char* msg);
int recvMsgLength(chat_args *args);
int recvMsg(chat_args *args, int messageLength, char** messageRecu);
void display(char* username, char* msg);
int sendMessage(chat_args *args, char* message);
int askUsername(chat_args *args);
void* reception(void* t);
void* saisie(void* t);
void launchChat(chat_args* args);

/* commands.c */
int checkCommand(char* msg);
int sendFile(chat_args* args);
void recvFile(chat_args* args);

/* connexion.c */
void createChat(chat_args* args, char* address, int port);
void shutdownClient(chat_args* args);

#endif