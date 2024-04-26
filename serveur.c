#include "serveur.h"

#define PORT 5001

int dS;
int NB_CLIENTS;
User *users;

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

int empty_client() {
  for (int i=0; i<NB_CLIENTS; i++) {
    if (users[i].ad == -1) {
      return i;
    }
  }
  return -1;
}

void broadcast(int index, int messageLength, char* msg) {
  for (int i=0; i < NB_CLIENTS; i++) {
    if (i!=index && users[i].ad != -1) {
      send(users[i].ad, &messageLength, sizeof(int), 0);
      send(users[i].ad, msg, messageLength*sizeof(char), 0);
    }
  }
}

void *transmission(void *t) {
  int user_index = (long)t;
  while(1) {
    // Reçoie de la taille du message
    int messageLength;
    if (recv(users[user_index].ad, &messageLength, sizeof(int), 0) > 0) {
      printf("Taille : %d\n", messageLength);
      char* msg = calloc(messageLength, sizeof(char));
      // Reçoie un message de l'envoyeur
      if (recv(users[user_index].ad, msg, messageLength*sizeof(char), 0) > 0) {
        printf("Message reçu Client %d: %s\n",user_index, msg);

        if (strcmp(msg, "fin\n") == 0) {
          break;
        }

        // Transmission du message vers le receveur
        broadcast(user_index, messageLength, msg);
      }
      free(msg);
    } else {
      break;
    }
  }
  shutdown((long)users[user_index].ad, 2);
  users[user_index].ad = -1;
  printf("Client %d disconnected\n", user_index);
  pthread_exit(0);
}

void connect_users() {
  while(1) {
    int index = empty_client();
    while(index > -1) {
      printf("\x1b[34m");
      socklen_t lg = sizeof(struct sockaddr_in) ;
      struct sockaddr_in aC;

      users[index].ad = accept(dS, (struct sockaddr*) &aC,&lg);
      printf("Client %d Connecté\n", index);
      pthread_create(&users[index].thread, 0, transmission, (void*)(long)index);

      index = empty_client();
    }
  }
}

int main(int argc, char *argv[]) {
  // Vérification des paramètres
  if(argc != 2){
    printf("Erreur: format de commande: ./serveur <NB_CLIENTS>");
    exit(EXIT_FAILURE);
  }
  NB_CLIENTS = atoi(argv[1]);
  users = (User *)malloc(NB_CLIENTS * sizeof(User));

  init();
  connect_users();
}