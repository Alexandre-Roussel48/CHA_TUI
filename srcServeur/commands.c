#include "headers/chat.h"

int checkCommand(char* msg, pthread_mutex_t mutex_lock) {
	char* msgCopy = (char*)calloc(strlen(msg)+1, sizeof(char));
	strcpy(msgCopy, msg);
    pthread_mutex_lock(&mutex_lock);
    char* msgTok = strtok(msgCopy, " ");
    pthread_mutex_unlock(&mutex_lock);

	if (strcmp(msgTok, "/commands\n") == 0) {free(msgCopy); return 0;}
	else if (strcmp(msgTok, "/members\n") == 0) {free(msgCopy); return 1;}
	else if (strcmp(msgTok, "/whisper") == 0) {free(msgCopy); return 2;}
	else if (strcmp(msgTok, "/kick") == 0) {free(msgCopy); return 3;}
    else if (strcmp(msgTok, "/bye\n") == 0) {free(msgCopy); return 4;}
	return -1;
}

void commands(int index, user* users) {
	char* list = "Commands :\n\t/commands : list all the commands\n\t/members : list all the members in chat\n\t/whisper <username> <message> : send a private message to someone\n\t/kick <username> : kick someone\n/bye : exit from chat\n";
	sendUsername(index, users, "");
	sendMsg(index, list, strlen(list) + 1, users);
}

void members(int index, int nbClients, user* users) {
    int totalLength = 0;
    for (int i = 0; i < nbClients; i++) {
        if (users[i].username != NULL) {totalLength += strlen(users[i].username) + 1;}
    }
    char* list = (char*)malloc((totalLength + 11) * sizeof(char));
    strcpy(list, "Members :\n\t");
    for (int i = 0; i < nbClients; i++) {
        if (users[i].username != NULL) {
            strcat(list, users[i].username);
            strcat(list, "\t");
        }
    }
    sendUsername(index, users, "");
    sendMsg(index, list, strlen(list) + 1, users);
    free(list);
}

void whisper(int index, char* msg, int msgLength, int nbClients, user* users, pthread_mutex_t mutex_lock) {
	char* msgCopy = (char*)calloc(msgLength, sizeof(char));
	strcpy(msgCopy, msg);

    pthread_mutex_lock(&mutex_lock);
    char* msgTok = strtok(msgCopy, " "); // "/whisper"
    char* username = strtok(NULL, " "); // "<username>"
    msgTok = strtok (NULL, "\n"); // "<message>"
    pthread_mutex_unlock(&mutex_lock);

    int whisperIndex = 0;
    while (whisperIndex < nbClients) {
        if (whisperIndex != index && users[whisperIndex].username != NULL) {
            char* usernameSliced = (char*)malloc(strlen(users[whisperIndex].username));
            strcpy(usernameSliced, users[whisperIndex].username);
            usernameSliced[strlen(usernameSliced)-1] = '\0';
            if (strcmp(username, usernameSliced) == 0) {break;}
            free(usernameSliced);
        }
        whisperIndex += 1;
    }

    if (whisperIndex < nbClients) {
        sendUsername(whisperIndex, users, users[index].username);
        char* msg = (char*)malloc((strlen(msgTok) + 13)*sizeof(char));
        strcpy(msg, msgTok);
        strcat(msg, " (whispered)\n");
        sendMsg(whisperIndex, msg, strlen(msg) + 1, users);
    } else {
        char* error = "User does not exist\n";
        sendUsername(index, users, "");
        sendMsg(index, error, strlen(error) + 1, users);
    }
    free(msgCopy);
}

void kick(int index, char* msg, int msgLength, int nbClients, user* users, pthread_mutex_t mutex_lock) {
    char* msgCopy = (char*)calloc(msgLength, sizeof(char));
    strcpy(msgCopy, msg);

    pthread_mutex_lock(&mutex_lock);
    char* username = strtok(msgCopy, " "); // "/kick"
    username = strtok(NULL, " "); // "<username>"
    pthread_mutex_unlock(&mutex_lock);

    int kickIndex = 0;
    while (kickIndex < nbClients) {
        if (kickIndex != index && users[kickIndex].username != NULL) {
            if (strcmp(username, users[kickIndex].username) == 0) {break;}
        }
        kickIndex += 1;
    }

    if (kickIndex < nbClients) {
        shutdownClient(kickIndex, users, mutex_lock);
    } else {
        char* error = "User does not exist\n";
        sendUsername(index, users, "");
        sendMsg(index, error, strlen(error) + 1, users);
    }
    free(msgCopy);
}