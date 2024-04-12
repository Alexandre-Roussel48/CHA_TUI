#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#define TAILLE_MESS 140
#define PORT 5000

int main(int argc, char *argv[]) {

  printf("Début programme\n");
  int dS = socket(PF_INET, SOCK_STREAM, 0);
  printf("Socket Créé\n");

  struct sockaddr_in aS;
  aS.sin_family = AF_INET;
  inet_pton(AF_INET,argv[1],&(aS.sin_addr)) ;
  aS.sin_port = htons(PORT) ;
  socklen_t lgA = sizeof(struct sockaddr_in) ;
  connect(dS, (struct sockaddr *) &aS, lgA) ;
  printf("Socket Connecté\n");

  int envoie = 1;
  while(envoie){

    // L'utilisateur 1 entre son message
    char* messageEnvoie = (char*)malloc(TAILLE_MESS);
    fgets(messageEnvoie, TAILLE_MESS*sizeof(char), stdin);

    send(dS, messageEnvoie, TAILLE_MESS*sizeof(char), 0);
    printf("Message Envoyé \n");
    if(strcmp(messageEnvoie, "fin\n") == 0){envoie = 0;}

    char* messageRecu = (char*)malloc(TAILLE_MESS);
    recv(dS, messageRecu, TAILLE_MESS, 0);
    printf("Réponse reçue : %s\n", messageRecu);
  }

  shutdown(dS,2);
  printf("Fin du programme");
}