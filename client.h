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
void init(char* address);
int find_first_slash(char *str);
void* saisie();
void* reception();

#endif