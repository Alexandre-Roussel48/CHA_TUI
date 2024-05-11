#include "./headers/chat.h"

/**
 * permet la création du socket
 * @params {char*} correspond à l'address du serveur
 * @return {int dS} retourne le descripteur du socket qui vient d'etre créé
*/
void createChat(chat_args* args, char* address, int port) {
  chat_args res;
  res.dS = socket(PF_INET, SOCK_STREAM, 0); // Création du socket pour le protocole TCP
  res.tailleMess = 256;

  struct sockaddr_in aS;
  aS.sin_family = AF_INET; // L'IP du serveur sera une IPv4
  inet_pton(AF_INET,address,&(aS.sin_addr)) ; // Permet de spécifier l'adresse du serveur sous forme binaire
  aS.sin_port = htons(port) ; // Permet de spécifier le port sûr lequel se connecter sous forme binaire
  socklen_t lgA = sizeof(struct sockaddr_in);

  if (connect(res.dS, (struct sockaddr *) &aS, lgA) < 0) { // test de la connexion au serveur
    perror("Connection failed");
    exit(EXIT_FAILURE);
  }

  *args = res;
}

int countLines(char* msg) {
  int lines = 0;
  while (*msg != '\0') {
      if (*msg == '\n') {
          lines++;
      }
      msg++;
  }
  int msgSize = strlen(msg) % 80;
  return (lines > msgSize) ? lines : msgSize;
}

int recvMessage(chat_args *args) {
  // reception username
  int messageLength;
  int res;

  res = recv(args->dS, &messageLength, sizeof(int), 0);
  if (res < 0) {
    perror("messageLength receive failed\n");
    return -1;
  } else if (res == 0 || messageLength == -1) {
    return 0;
  }

  char* username = (char*)malloc(messageLength);
  res = recv(args->dS, username, messageLength*sizeof(char), 0);
  // gestion erreur de la reception du message
  if (res < 0) {
    free(username);
    perror("message receive failed\n");
    return -1;
  } else if (res == 0) {
    free(username);
    return 0;
  }
  if (messageLength > 1) {
    username[messageLength-2] = '\0';
  } else {
    username = "";
  }

  // reception taille message
  res = recv(args->dS, &messageLength, sizeof(int), 0);
  // gestion erreur de la reception de la taille
  if (res < 0) {
    perror("messageLength receive failed\n");
    return -1;
  } else if (res == 0) {
    return 0;
  }

  // reception message
  char* messageRecu = (char*)malloc(messageLength);
  res = recv(args->dS, messageRecu, messageLength*sizeof(char), 0);
  // gestion erreur de la reception du message
  if (res < 0) {
    free(messageRecu);
    perror("message receive failed\n");
    return -1;
  } else if (res == 0) {
    free(messageRecu);
    return 0;
  }
  int nbLignes = countLines(messageRecu);
  printf("\033[s\033[%dL\t/%s> %s\033[u\033[%dB", nbLignes, username, messageRecu, nbLignes); // affichage plus joli
  fflush(stdout); // force la maj de la sortie standard
  if (strcmp(username, "") != 0) {free(username);}
  free(messageRecu);
  return 1;
}

/**
 * fonction qui permet d'envoyer : taille message + message
*/
int sendMessage(chat_args *args, char* message) {
  int messageLength = strlen(message)+1;
  // envoie de la taille du message
  if (send(args->dS, &messageLength, sizeof(int), 0) < 0) {
    perror("messageLength send failed\n");
    return -1;
  }
  // envoie du message
  if (send(args->dS, message, messageLength*sizeof(char), 0) < 0) {
    perror("message send failed\n");
    return -1;
  }
  return 1;
}

/**
 * permet de demander le username du client
 * @return{int} si erreur lors de l'envoie du nom d'utilisateur renvoie -1
 *              si aucune erreurs lors de l'envoie renvoie 1
*/
int askUsername(chat_args *args) {
  char* messageEnvoie = (char*)malloc(args->tailleMess);
  printf("\t> Veuillez entrer votre Username :\n");
  printf("\t> ");
  fgets(messageEnvoie, args->tailleMess, stdin);
  if (sendMessage(args, messageEnvoie) < 0) {
    perror("Username binding failed\n");
    return -1;
  }
  return 1;
}

/**
 * permet au client de lire un message recu
 * est utilisé dans un thread
 * @return {void*}
*/
void* reception(void* t){
  chat_args* args = (chat_args*)t;
  int res = 1;
  while(res == 1){
    res = recvMessage(args);
  }
  pthread_exit(0);
}

/**
 * permet au client de saisir un message
 * est utilisé dans un thread
 * @return {void*}
*/
void* saisie(void* t){
  chat_args* args = (chat_args*)t;
  int res=1;
  while(res == 1){
    char* messageEnvoie = malloc(args->tailleMess);
    printf("\t> ");
    fgets(messageEnvoie, args->tailleMess, stdin);

    res = sendMessage(args, messageEnvoie);
    
    free(messageEnvoie);
  }
  pthread_exit(0);
}

int launchChat(chat_args* args) {
  chat_args res = *args;
  printf("\x1b[34m"); // changement de couleur du texte pour la connexion

  // demande du nom d'utilisateur
  if (askUsername(args) < 0) {return -1;}

  printf("\x1b[32m\n"); // changement de couleur du texte pour la discussion

  pthread_create(&res.tsaisie, NULL, saisie, (void*)args); // Envoie d'un message
  pthread_create(&res.treception, NULL, reception, (void*)args); // Reception d'un message

  pthread_join(res.treception, 0);

  *args = res;
  return -1;
}