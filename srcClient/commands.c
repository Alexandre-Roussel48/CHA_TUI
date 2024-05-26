#include "headers/chat.h"

int checkCommand(char* msg) {
	char* msgCopy = (char*)calloc(strlen(msg)+1, sizeof(char));
	strcpy(msgCopy, msg);
    char* msgTok = strtok(msgCopy, " ");

	if (strcmp(msgTok, "/sendFile\n") == 0) {free(msgCopy); return 0;}
    else if (strcmp(msgTok, "/recvFile\n") == 0) {free(msgCopy); return 1;}
	return -1;
}

int sendFile(chat_args* args) {
    FILE *fp;
    char path[1035];
    char *files[100];

	fp = popen("ls filesClient", "r");
    if (fp == NULL) {
        printf("\tFailed to run command\n" );
        return -1;
    }

    int file_count = 0;
	while (fgets(path, sizeof(path), fp) != NULL) {
        path[strcspn(path, "\n")] = '\0';
        files[file_count] = strdup(path);
        printf("\t%d > %s\n", file_count, files[file_count]);
        file_count++;
        if (file_count >= 100) {
            printf("Too many files, only listing the first 100 files.\n");
            break;
        }
    }
    pclose(fp);

    if (file_count == 0) {
        printf("No files to send.\n");
        return -1;
    }

    int fileNumber;
    printf("\tSelect number of file to send : ");
    scanf("%d", &fileNumber);

    while (getchar() != '\n');

    if (fileNumber < 0 || fileNumber >= file_count) {
        printf("Invalid file number selected.\n");
        return -1;
    }

    char *selectedFile = strdup(files[fileNumber]);

    for (int i = 0; i < file_count; i++) {
        free(files[i]);
    }

    printf("\tSending file %s to server\n", selectedFile);

    char *command = (char *)malloc(strlen("/sendFile ") + strlen(selectedFile) + 1);
    sprintf(command, "/sendFile %s", selectedFile);
    sendMessage(args, command);

    char *filePath = (char *)malloc(strlen("filesClient/") + strlen(selectedFile) + 1);
    sprintf(filePath, "filesClient/%s", selectedFile);
    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return -1;
    }

    char buffer[1024];
    while (fgets(buffer, 1024, file) != NULL) {
        sendMessage(args, buffer);
    }

    fclose(file);

    int messageLength = -1;
    send(args->dS, &messageLength, sizeof(int), 0);
    printf("\tFile sent to server\n");

    return 1;
}

void recvFile(chat_args* args) {
    int filenameLength;
    if ((filenameLength = recvMsgLength(args)) < 0) {
        if (filenameLength == -2) {display("", "There is no file in server\n");}
        else if (filenameLength == -3) {display("", "The number you sent is incorrect\n");}
        return;
    }

    char* filename;
    if (recvMsg(args, filenameLength, &filename) < 0) {
        return;
    }

    char *touch = (char *)malloc(strlen("touch filesClient/") + strlen(filename) + 1);
    sprintf(touch, "touch filesClient/%s", filename);
    system(touch);

    do {
        int msgLength;
        if ((msgLength = recvMsgLength(args)) < 0) {break;}
        char* msg;
        if (recvMsg(args, msgLength, &msg) <= 0) {break;}
        char *printf = (char *)malloc(strlen("printf \"") + strlen(msg) + 1 + strlen("\" >> filesClient/") + strlen(filename) + 1);
        sprintf(printf, "printf \"%s\" >> filesClient/%s", msg, filename);
        system(printf);
        free(printf);
    } while (1);
    display("", "File received\n");
    free(filename);
    free(touch);
}