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
  char msg[140];
  strcpy(msg, "fin\n");
  send(dS, msg, TAILLE_MESS*sizeof(char), 0);
  shutdown(dS, 2);
  exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {

  printf("\x1b[34m");
  dS = socket(PF_INET, SOCK_STREAM, 0);
  printf("Socket Créé\n");
  signal(SIGINT, sigint_handler);

  struct sockaddr_in aS;
  aS.sin_family = AF_INET;
  inet_pton(AF_INET,argv[1],&(aS.sin_addr)) ;
  aS.sin_port = htons(PORT) ;
  socklen_t lgA = sizeof(struct sockaddr_in) ;
  if (connect(dS, (struct sockaddr *) &aS, lgA) < 0) {
    perror("Connection failed");
    exit(1);
  }
  printf("Socket Connecté\n");
  printf("\x1b[32m\n");

  while(1){
    printf("\t> ");
    // L'utilisateur 1 entre son message
    char* messageEnvoie = (char*)malloc(TAILLE_MESS);
    fgets(messageEnvoie, TAILLE_MESS, stdin);

    send(dS, messageEnvoie, TAILLE_MESS*sizeof(char), 0);
    if(strcmp(messageEnvoie, "fin\n") == 0){break;}

    char* messageRecu = (char*)malloc(TAILLE_MESS);
    if (recv(dS, messageRecu, TAILLE_MESS*sizeof(char), 0) == 0) {
      break;
    }
    printf("\t/> %s", messageRecu);
  }

  shutdown(dS,2);
  printf("\x1b[34m");
  printf("Fin du chat\n");
  printf("\x1b[0m");
}