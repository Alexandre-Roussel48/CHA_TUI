#ifndef SERVEUR_CHAT
#define SERVEUR_CHAT

#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

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

/* connexion.c */
void createChat(int nbClients, int port, chat_args* args);
int acceptUser(chat_args* args);
void shutdownClient(int index, user* users, pthread_mutex_t mutex_lock);
void shutdownServer(chat_args* args);

/* chat.c */
void launchChat(int index, chat_args* args);
void* transmission(void *args);
int recvMsgLength(int index, user* users);
int recvMsg(int index, int msgLength, char** msg, user* users);
int sendUsername(int index, user* users, char* username);
int sendMsg(int index, char* msg, int msgLength, user* users);
void broadcast(int index, char* msg, int msgLength, chat_args* args);

/* commands.c */
int checkCommand(char* msg, pthread_mutex_t mutex_lock);
void commands(int index, user* users);
void members(int index, int nbClients, user* users);
void whisper(int index, char* msg, int msgLength, int nbClients, user* users, pthread_mutex_t mutex_lock);
void kick(int index, char* msg, int msgLength, int nbClients, user* users, pthread_mutex_t mutex_lock);

#endif