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
    else if (strcmp(msgTok, "/sendFile") == 0) {free(msgCopy); return 5;}
    else if (strcmp(msgTok, "/listFiles\n") == 0) {free(msgCopy); return 6;}
    else if (strcmp(msgTok, "/recvFile") == 0) {free(msgCopy); return 7;}
	return -1;
}

void commands(int index, user* users) {
	char* list = "Commands :\n\t/commands : list all the commands\n\t/members : list all the members in chat\n\t/whisper <username> <message> : send a private message to someone\n\t/kick <username> : kick someone\n\t/bye : exit from chat\n";
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

void recvFile(int index, char* msg, int msgLength, user* users, pthread_mutex_t mutex_lock) {
    char* msgCopy = (char*)calloc(msgLength, sizeof(char));
    strcpy(msgCopy, msg);

    pthread_mutex_lock(&mutex_lock);
    char* file = strtok(msgCopy, " "); // "/sendFile"
    file = strtok(NULL, " "); // "<file>"
    pthread_mutex_unlock(&mutex_lock);

    char *touch = (char *)malloc(strlen("touch filesServer/") + strlen(file) + 1);
    sprintf(touch, "touch filesServer/%s", file);
    system(touch);

    do {
        int msgLength;
        if ((msgLength = recvMsgLength(index, users)) < 0) {break;}
        char* msg;
        if (recvMsg(index, msgLength, &msg, users) <= 0) {break;}
        char *printf = (char *)malloc(strlen("printf \"") + strlen(msg) + 1 + strlen("\" >> filesServer/") + strlen(file) + 1);
        sprintf(printf, "printf \"%s\" >> filesServer/%s", msg, file);
        system(printf);
        free(printf);
    } while (1);
    free(msgCopy);
    free(touch);
}

void listFiles(int index, user* users) {
    FILE *fp;
    char path[1035];
    char* result = malloc(4096);

    fp = popen("ls filesServer", "r");
    if (fp == NULL) {
        printf("\tFailed to run command\n" );
        return;
    }

    strcat(result, "\n");
    int count = 0;
    while (fgets(path, sizeof(path), fp) != NULL) {
        path[strcspn(path, "\n")] = '\0';

        char line[1050];
        snprintf(line, sizeof(line), "\t%d > %s\n", count, path);
        strcat(result, line);
        count++;
    }

    pclose(fp);

    if (result[0] == '\0') {
        sendUsername(index, users, "");
        sendMsg(index, "There is no file on server\n", 28, users);
    } else {
        sendUsername(index, users, "");
        sendMsg(index, result, strlen(result) + 1, users);
    }
    free(result);
}

void sendFile(int index, char* msg, int msgLength, user* users, pthread_mutex_t mutex_lock) {
    char* msgCopy = (char*)calloc(msgLength, sizeof(char));
    strcpy(msgCopy, msg);

    pthread_mutex_lock(&mutex_lock);
    char* fileNumber = strtok(msgCopy, " "); // "/recvFile"
    fileNumber = strtok(NULL, " "); // "<number of file>"
    pthread_mutex_unlock(&mutex_lock);

    fileNumber[1] = '\0';
    int fileIndex = atoi(fileNumber);

    FILE *fp;
    char path[1035];
    char *files[100];

    fp = popen("ls filesServer", "r");
    if (fp == NULL) {
        return;
    }

    int file_count = 0;
    while (fgets(path, sizeof(path), fp) != NULL) {
        path[strcspn(path, "\n")] = '\0';
        files[file_count] = strdup(path);
        file_count++;
        if (file_count >= 100) {
            break;
        }
    }
    pclose(fp);

    sendUsername(index, users, "");
    sendMsg(index, "/recvFile\n", 11, users);

    if (file_count == 0) {
        int errorCode = -2; // no file in server
        send(users[index].ad, &errorCode, sizeof(int), 0);
        return;
    }

    if (fileIndex < 0 || fileIndex >= file_count) {
        int errorCode = -3; // number is not in list
        send(users[index].ad, &errorCode, sizeof(int), 0);
        return;
    }

    char *selectedFile = strdup(files[fileIndex]);

    for (int i = 0; i < file_count; i++) {
        free(files[i]);
    }

    sendMsg(index, selectedFile, strlen(selectedFile) + 1, users);

    char *filePath = (char *)malloc(strlen("filesServer/") + strlen(selectedFile) + 1);
    sprintf(filePath, "filesServer/%s", selectedFile);
    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    char buffer[1024];
    while (fgets(buffer, 1024, file) != NULL) {
        sendMsg(index, buffer, strlen(buffer) + 1, users);
    }

    fclose(file);

    int messageLength = -1;
    send(users[index].ad, &messageLength, sizeof(int), 0);
}