#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#define TAILLE_MESS 140
#define PORT 5001

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

  while(1) {
    socklen_t lg = sizeof(struct sockaddr_in) ;

    struct sockaddr_in aC1 ;
    int dSC1 = accept(dS, (struct sockaddr*) &aC1,&lg) ;
    printf("\x1b[34m");
    printf("Client 1 Connecté\n");

    struct sockaddr_in aC2 ;
    int dSC2 = accept(dS, (struct sockaddr*) &aC2,&lg) ;
    printf("Client 2 Connecté\n");
    printf("\x1b[0m");

    char* msg = calloc(TAILLE_MESS, sizeof(char));
    while (1) {
      // Reçoie un message du client 1
      if (recv(dSC1, msg, TAILLE_MESS*sizeof(char), 0) > 0) {
        printf("Message reçu Client 1: %s\n", msg);

        if (strcmp(msg, "fin\n") == 0) {
          break;
        }

        // Transmission du message vers le client 2
        send(dSC2, msg, TAILLE_MESS*sizeof(char), 0);
      }

      // Reçoie un message du client 2
      if (recv(dSC2, msg, TAILLE_MESS*sizeof(char), 0) > 0) {
        printf("Message reçu Client 2: %s\n", msg);

        if (strcmp(msg, "fin\n") == 0) {
          break;
        }

        // Transmission du message vers le client 1
        send(dSC1, msg, TAILLE_MESS*sizeof(char), 0);
      }
    }
    free(msg);
    shutdown(dSC1, 2);
    shutdown(dSC2, 2);
  }
}