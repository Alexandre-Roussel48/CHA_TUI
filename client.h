#ifndef CLIENT
#define CLIENT

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#define TAILLE_MESS 256
#define PORT 5001

void sigint_handler(int signal);
int ask_username();
int send_message(char* message);
int rcv_message();
void* saisie();
void* reception();
void init(char* address);

#endif