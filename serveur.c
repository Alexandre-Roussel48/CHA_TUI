#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define TAILLE_MESS 140
#define PORT 5000

int dS;

void sigint_handler(int signal) {
  shutdown(dS, 2);
  exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
  
  printf("\x1b[32m");
  printf("Début programme\n");

  dS = socket(PF_INET, SOCK_STREAM, 0);
  printf("Socket Créé\n");
  signal(SIGINT, sigint_handler);

  struct sockaddr_in ad;
  ad.sin_family = AF_INET;
  ad.sin_addr.s_addr = INADDR_ANY ;
  ad.sin_port = htons(PORT) ;
  if (bind(dS, (struct sockaddr*)&ad, sizeof(ad)) < 0) {
    perror("Bind failed");
    exit(1);
  }
  printf("Socket Nommé\n");

  listen(dS, 2) ;
  printf("Mode écoute\n\n");
  printf("\x1b[0m");

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
      // Receive of Client 1 message
      if (recv(dSC1, msg, TAILLE_MESS*sizeof(char), 0) > 0) {
        printf("Message reçu Client 1: %s\n", msg);

        if (strcmp(msg, "fin\n") == 0) {
          break;
        }

        // Send of Client 1 message to Client 2
        send(dSC2, msg, TAILLE_MESS*sizeof(char), 0);
      }

      // Receive of Client 2 message
      if (recv(dSC2, msg, TAILLE_MESS*sizeof(char), 0) > 0) {
        printf("Message reçu Client 2: %s\n", msg);

        if (strcmp(msg, "fin\n") == 0) {
          break;
        }

        // Send of Client 2 message to Client 1
        send(dSC1, msg, TAILLE_MESS*sizeof(char), 0);
      }
    }
    free(msg);
    shutdown(dSC1, 2);
    shutdown(dSC2, 2);
  }
}