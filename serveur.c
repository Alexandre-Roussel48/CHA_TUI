#include "serveur.h"

#define PORT 5001

int dS;
int NB_CLIENTS;   // Definie le nombre de clients
User *users;      // tableau d'utilisateurs
pthread_mutex_t mutex_lock;

void init() {
  for(int i=0; i<NB_CLIENTS; i++){
    users[i].ad = -1; // Initialisation du tableau d'adresse
  }

  printf("\x1b[32m");// Permet de mettre le texte en couleur
  printf("Début programme\n");
  dS = socket(PF_INET, SOCK_STREAM, 0); // Initialisation du descripteur de la socket
  printf("Socket Créé\n");

  struct sockaddr_in ad;
  ad.sin_family = AF_INET; // L'IP du serveur sera une IPv4
  ad.sin_addr.s_addr = INADDR_ANY; // Permet d'écouter toutes les adresses
  ad.sin_port = htons(PORT); // Permet de spécifier le port sûr lequel se connecter sous forme binaire

  int optval = 1;
  setsockopt(dS, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)); // Permet de réutiliser un socket
  
  if (bind(dS, (struct sockaddr*)&ad, sizeof(ad)) < 0) {
    perror("Bind failed");
    exit(EXIT_FAILURE);
  }
  printf("Socket Nommé\n");


  listen(dS, 2) ;
  printf("Mode écoute\n\n");
  printf("\x1b[0m"); // Permet de changer la couleur du texte
}

/**
 * Regarde la premier place libre dans le tableau de Users
 * @return {int} index de la place libre dans le tableau
*/
int empty_client() {
  for (int i=0; i<NB_CLIENTS; i++) {
    if (users[i].ad == -1) {
      return i;
    }
  }
  return -1;
}

int send_message(int index, char* message, int messageLength) {
  if (send(users[index].ad, &messageLength, sizeof(int), 0) < 0) {
    perror("messageLength send failed\n");
    return -1;
  }
  if (send(users[index].ad, message, messageLength*sizeof(char), 0) < 0) {
    perror("message send failed\n");
    return -1;
  }
  return 1;
}

/**
 * Envoie le message et la taille du message à tous les utilisateurs
 * @params{int, int, char*}
 *    - index de l'utilisateur dans le tableau (int)
 *    - taille du message (int)
 *    - message (char*)
*/
void broadcast(int index, char* msg, int messageLength) {
  for (int i=0; i < NB_CLIENTS; i++) {
    if (i!=index && users[i].ad != -1) {
      send_message(i,msg,messageLength);
    }
  }
}

int recv_message_length(int index) {
  int messageLength;
  int res = recv(users[index].ad, &messageLength, sizeof(int), 0); // reception taille message
  if (res < 0) { // gestion erreur
    perror("messageLength receive failed\n");
    return -1;
  } else if (res == 0) {
    return 0;
  }
  printf("Taille : %d\n", messageLength);
  return messageLength;
}

int recv_message(int index, int messageLength, char** message) {
  char* messageRecu = (char*)malloc(messageLength);
  int res = recv(users[index].ad, messageRecu, messageLength*sizeof(char), 0);
  if (res < 0) {
    free(messageRecu);
    perror("message receive failed\n");
    return -1;
  } else if (res == 0) {
    free(messageRecu);
    return 0;
  }
  if (users[index].username==NULL) {
    users[index].username = messageRecu;
    printf("%s Connecté\n", users[index].username);
    return 2;
  }
  printf("Message reçu Client %d: %s\n",index, messageRecu);
  *message = messageRecu;
  return 1;
}

/**
 * transmet un message recu vers un client
 * @param{void* t} correspond à l'index du client
*/
void* transmission(void *t) {
  int user_index = (long)t;
  while(1) {
    int messageLength = recv_message_length(user_index);
    if (messageLength <= 0) {
      goto shutdown;
    }
    char* messageRecu;
    int res = recv_message(user_index, messageLength, &messageRecu);
    if (res <= 0) {
      goto shutdown;
    } else if (res == 1) {
      if (strcmp(messageRecu, "fin\n") == 0) {
        goto shutdown;
      }
      broadcast(user_index, messageRecu, messageLength);
    }
  }

  shutdown:
  shutdown((long)users[user_index].ad, 2);
  pthread_mutex_lock(&mutex_lock);
  users[user_index].ad = -1;
  pthread_mutex_unlock(&mutex_lock);
  printf("Client %d disconnected\n", user_index);
  pthread_exit(0);
}

/**
 * permet de connecter un client en lui demandant son username
*/
void connect_users() {
  while(1) {
    int index = empty_client(); // recupere l'index du nouveau client dans le tableau de clients

    while(index > -1) { // Continue a accepter tant qu'il y a de la place

      printf("\x1b[34m");

      socklen_t lg = sizeof(struct sockaddr_in) ;
      struct sockaddr_in aC;
      users[index].ad = accept(dS, (struct sockaddr*) &aC,&lg); // On connecte la connection avec le client et on stock l'adresse

      pthread_create(&users[index].thread, 0, transmission, (void*)(long)index);

      index = empty_client();
    }
  }
}

int main(int argc, char *argv[]) {
  // Vérification des paramètres
  if(argc != 2){
    printf("Erreur: format de commande: ./serveur <NB_CLIENTS>\n");
    exit(EXIT_FAILURE);
  }
  NB_CLIENTS = atoi(argv[1]);
  users = (User *)malloc(NB_CLIENTS * sizeof(User));

  init();
  connect_users();
}