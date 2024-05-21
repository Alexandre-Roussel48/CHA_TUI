#include "headers/chat.h"

int checkCommand(char* msg, pthread_mutex_t mutex_lock) {
	char* msgCopy = (char*)calloc(strlen(msg)+1, sizeof(char));
	strcpy(msgCopy, msg);
    pthread_mutex_lock(&mutex_lock);
    char* msgTok = strtok(msgCopy, " ");
    pthread_mutex_unlock(&mutex_lock);

	if (strcmp(msgTok, "/commands\n") == 0) {free(msgCopy); return 0;}
    else if (strcmp(msgTok, "/files\n") == 0) {free(msgCopy); return 1;}
    else if (strcmp(msgTok, "/file\n") == 0) {free(msgCopy); return 2;}
	return -1;
}

