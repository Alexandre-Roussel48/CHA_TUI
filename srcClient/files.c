#include "headers/chat.h"

/**
 * 
*/
int connectFileServer(chat_args* args) {
    struct sockaddr_in aS;
    aS.sin_family = AF_INET; // L'IP du serveur sera une IPv4
    inet_pton(AF_INET,args->address,&(aS.sin_addr)) ; // Permet de spécifier l'adresse du serveur sous forme binaire
    aS.sin_port = htons(args->portFileServer) ; // Permet de spécifier le port sûr lequel se connecter sous forme binaire
    socklen_t lgA = sizeof(struct sockaddr_in);

    int dS = socket(PF_INET, SOCK_STREAM, 0); // Création du socket pour le protocole TCP
    if (connect(dS, (struct sockaddr *) &aS, lgA) < 0) { // test de la connexion au serveur
    perror("Connection failed");
    exit(EXIT_FAILURE);
    }

    return dS;
}

/**
 * @brief Thread function to send a file to the server.
 * 
 * @param args Pointer to the chat_args structure.
*/
void* sendFileThread(void* t) {
    // on définit les arguments de la fonction
    typedef struct {
        chat_args* args;
        char* filename;
    } transargs;
    transargs* trans = (transargs*)t;
    chat_args* args = trans->args;
    char* filename = trans->filename;

    // connexion du client au serveur de fichier
    int dS = connectFileServer(args);

    // envoie du nom du fichier
    int filenameLength = strlen(filename);
    if(send(dS, &filenameLength, sizeof(int), 0) < 0) {pthread_exit(0);}
    if (send(dS, filename, strlen(filename)*sizeof(char), 0) < 0) {pthread_exit(0);}

    FILE *filePointer;
    const char *directory = "filesClient";
    char* filepath = (char*)malloc(sizeof(char)*(strlen(directory)+strlen(filename)+2));
    snprintf(filepath, sizeof(char)*(strlen(directory)+strlen(filename)+2), "%s/%s", directory, filename);

    filePointer = fopen(filepath, "rb");
    if (filePointer == NULL) {pthread_exit(0);}
    fseek(filePointer, 0, SEEK_END); // déplace le curseur à la fin du fichier
    int fileLength = ftell(filePointer); // donne la position du curseur dans le fichier
    if (send(dS, &fileLength, sizeof(int), 0) < 0) {pthread_exit(0);}
    fseek(filePointer, 0, SEEK_SET); // remet le curseur au début du fichier

    const size_t chunkSize = 256;
    char buffer[chunkSize];
    
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, chunkSize, filePointer)) > 0) {
        if (send(dS, &bytesRead, sizeof(int), 0) < 0) {pthread_exit(0);}
        if (send(dS, buffer, bytesRead, 0) < 0) {pthread_exit(0);}
    }

    fclose(filePointer);
    display("", "File sent\n");
    fflush(stdout);

    shutdown(dS, 2);
    pthread_exit(0);
}

/**
 * Sends a file to the server.
 * 
 * @param args Pointer to the chat_args structure.
 */
void sendFile(chat_args* args) {
    struct dirent *entry;
    DIR *dr = opendir("filesClient"); 
  
    if (dr == NULL) { 
        printf("Could not open source directory" ); 
        return;
    } 
 
    int i = 0;
    while ((entry = readdir(dr)) != NULL) { // équivaut à command ls
        if (entry->d_type == DT_REG) { /* If the entry is a regular file */
            printf("\t%d : %s\n", i, entry->d_name);
            i++;
        }
    }

    rewinddir(dr); // remet le pointeur de lecture au début du répertoire

    int clientEntry;
    printf("\tWhich file do you want to send ? (enter the number) : ");
    scanf("%d", &clientEntry);
    while(getchar() != '\n'); // pour vider stdin à cause du scanf

    typedef struct {
        chat_args* args;
        char* filename;
    } transargs;

    transargs* t = (transargs*)malloc(sizeof(transargs));
    t->args = args;

    i = 0;
    while ((entry = readdir(dr)) != NULL) { // équivaut à command ls
        if (entry->d_type == DT_REG) { /* If the entry is a regular file */
            if(i == clientEntry) {
                printf("\tSending file %s\n", entry->d_name);
                t->filename = (char*)malloc(sizeof(char)*(strlen(entry->d_name)+1));
                strcpy(t->filename, entry->d_name);
                break;
            }
            i++;
        }
    }

    closedir(dr);

    pthread_t thread;
    pthread_create(&thread, 0, sendFileThread, (void*)t);
}

/**
 * Receives a message from a file client.
 * 
 * @param dS socket descriptor of file client.
 * @param msg Pointer to the buffer to store the received message.
 * @return Length of the received message or -1 on failure.
 */
int receiveFileMessage(int dS, char** msg) {
    int msgLength;
    if (recv(dS, &msgLength, sizeof(int), 0) <= 0) {return -1;}
    char* msgRecv = (char*)calloc(msgLength, sizeof(char));
    if (recv(dS, msgRecv, msgLength*sizeof(char), 0) <= 0) {free(msgRecv); return -1;}

    *msg = msgRecv;
    return msgLength;
}

/**
 * @brief Receive a file from the server.
*/
void* receiveFileThread(void* t) {
    // on définit les arguments de la fonction
    typedef struct {
        chat_args* args;
        char* filename;
    } transargs;
    transargs* trans = (transargs*)t;
    chat_args* args = trans->args;
    char* filename = trans->filename;

    int dS = connectFileServer(args);

    // envoie du nom du fichier
    int filenameLength = strlen(filename) + 1;
    if(send(dS, &filenameLength, sizeof(int), 0) < 0) {pthread_exit(0);}
    if (send(dS, filename, strlen(filename)*sizeof(char), 0) < 0) {pthread_exit(0);}

    printf("Receiving %s\n", filename);

    // reception du de la taille du fichier
    const char *directory = "filesClient";
    char* filepath = (char*)malloc(sizeof(char)*(strlen(directory)+strlen(filename)+2));
    snprintf(filepath, sizeof(char)*(strlen(directory)+strlen(filename)+2), "%s/%s", directory, filename);

    FILE *filePointer;
    filePointer = fopen(filepath, "wb");
    if (filePointer == NULL) {pthread_exit(0);}

    int fileLength;
    if (recv(dS, &fileLength, sizeof(int), 0) <= 0) {pthread_exit(0);}

    // écriture du fichier
    int wrote = 0;
    while (wrote < fileLength) {
        char* bloc;
        int blocLength = receiveFileMessage(dS, &bloc);
        fwrite(bloc, sizeof(char), blocLength, filePointer);
        wrote += blocLength;
    }

    fclose(filePointer);
    printf("File %s received\n", filename);

    shutdown(dS,2);
    pthread_exit(0);
}

/**
 * Receives a file from the server.
 * 
 * @param server Pointer to the chat_args structure.
 */
void recvFile(chat_args* args) {

    int total;
    total = recv(args->dS, &total, sizeof(int), 0);
    char** filenames = (char**)malloc(sizeof(char*)*total);
    for(int i=0; i<total; i++) {
        receiveFileMessage(args->dS, &filenames[i]);
        printf("\t%d : %s\n", i, filenames[i]);
    }

    int clientEntry;
    printf("\tQuel fichier voulez-vous recevoir ? (entrez le nombre) : ");
    scanf("%d", &clientEntry);
    while(getchar() != '\n'); // pour vider stdin à cause du scanf

    typedef struct {
        chat_args* args;
        char* filename;
    } transargs;

    transargs* t = (transargs*)malloc(sizeof(transargs));
    t->args = args;
    t->filename = filenames[clientEntry];

    pthread_t thread;
    pthread_create(&thread, 0, receiveFileThread, (void*)t);
}