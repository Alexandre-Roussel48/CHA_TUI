#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define PORT 5001
#define NB_CLIENTS 2

int addresses[NB_CLIENTS] = {-1,-1};

int empty_client() {
  for (int i=0; i<NB_CLIENTS; i++) {
    if (addresses[i] == -1) {
      return i;
    }
  }
  return -1;
}

void *transmission(void *t) {
  int i = (long)t;
  int receiver = (i+1)%2;
  while(1) {
    // Reçoie de la taille du message
    int messageLength;
    if (recv(addresses[i], &messageLength, sizeof(int), 0) > 0) {
      printf("Taille : %d\n", messageLength);
      char* msg = calloc(messageLength, sizeof(char));
      // Reçoie un message de l'envoyeur
      if (recv(addresses[i], msg, messageLength*sizeof(char), 0) > 0) {
        printf("Message reçu Client %d: %s\n",i, msg);

        if (strcmp(msg, "fin\n") == 0) {
          break;
        }

        // Transmission du message vers le receveur
        if (addresses[receiver] != -1) {
          send(addresses[receiver], &messageLength, sizeof(int), 0);
          send(addresses[receiver], msg, messageLength*sizeof(char), 0);
        }
      }
      free(msg);
    }
  }
  shutdown((long)addresses[i], 2);
  addresses[i] = -1;
  printf("Client %d disconnected\n", i);
  pthread_exit(0);
}

int main(int argc, char *argv[]) {
  // Initialisation du tableau d'adresse
  for(int i=0; i<NB_CLIENTS; i++){
    addresses[i] = -1;
  }

  printf("\x1b[32m");// Permet de mettre le texte en couleur
  printf("Début programme\n");
  int dS = socket(PF_INET, SOCK_STREAM, 0); // Initialisation du descripteur de la socket
  printf("Socket Créé\n");

  struct sockaddr_in ad;
  ad.sin_family = AF_INET; // L'IP du serveur sera une IPv4
  ad.sin_addr.s_addr = INADDR_ANY; // Permet d'écouter toutes les adresses
  ad.sin_port = htons(PORT); // Permet de spécifier le port sûr lequel se connecter sous forme binaire

  // Permet de réutiliser un socket
  int optval = 1;
  setsockopt(dS, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
  
  if (bind(dS, (struct sockaddr*)&ad, sizeof(ad)) < 0) {
    perror("Bind failed");
    exit(1);
  }
  printf("Socket Nommé\n");

  listen(dS, 2) ;
  printf("Mode écoute\n\n");
  printf("\x1b[0m"); // Permet de changer la couleur du texte

  pthread_t threads[2];

  while(1) {
    int index = empty_client();
    while(index > -1) {
      printf("\x1b[34m");
      socklen_t lg = sizeof(struct sockaddr_in) ;
      struct sockaddr_in aC;

      addresses[index] = accept(dS, (struct sockaddr*) &aC,&lg);
      printf("Client %d Connecté\n", index);
      pthread_create(&threads[index], 0, transmission, (void*)(long)index);

      index = empty_client();
    }
  }
}