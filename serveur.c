#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define TAILLE_MESS 140
#define PORT 5001

int addresses[2];

void *transmission(void *t) {
  char* msg = calloc(TAILLE_MESS, sizeof(char));
  int i = (long)t;
  int receiver = (i+1)%2;
  while(1) {
    // Reçoie un message de l'envoyeur
    if (recv(addresses[i], msg, TAILLE_MESS*sizeof(char), 0) > 0) {
      printf("Message reçu Client %d: %s\n",(i+1), msg);

      if (strcmp(msg, "fin\n") == 0) {
        free(msg);
        break;
      }

      // Transmission du message vers le receveur
      send(addresses[receiver], msg, TAILLE_MESS*sizeof(char), 0);
    } else {
      free(msg);
      pthread_exit(0);
    }
  }
  shutdown(addresses[0], 2);
  shutdown(addresses[1], 2);
  pthread_exit(0);
}

int main(int argc, char *argv[]) {
  
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
    socklen_t lg = sizeof(struct sockaddr_in) ;

    struct sockaddr_in aC1 ;
    addresses[0] = accept(dS, (struct sockaddr*) &aC1,&lg) ;
    printf("\x1b[34m");
    printf("Client 1 Connecté\n");
    pthread_create(&threads[0], 0, transmission, (void*)0);

    struct sockaddr_in aC2 ;
    addresses[1] = accept(dS, (struct sockaddr*) &aC2,&lg) ;
    printf("Client 2 Connecté\n");
    printf("\x1b[0m");
    pthread_create(&threads[1], 0, transmission, (void*)1);

    pthread_join(threads[0], 0);
    pthread_join(threads[1], 0);
  }
}