#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#define TAILLE_MESS 140

int main(int argc, char *argv[]) {

  printf("Début programme\n");
  int dS = socket(PF_INET, SOCK_STREAM, 0);
  printf("Socket Créé\n");

  struct sockaddr_in aS;
  aS.sin_family = AF_INET;
  inet_pton(AF_INET,argv[1],&(aS.sin_addr)) ;
  aS.sin_port = htons(atoi(argv[2])) ;
  socklen_t lgA = sizeof(struct sockaddr_in) ;
  connect(dS, (struct sockaddr *) &aS, lgA) ;
  printf("Socket Connecté\n");

  int envoie = 1;
  while(envoie){
    char* messageRecu = (char*)malloc(TAILLE_MESS);
    recv(dS, messageRecu, TAILLE_MESS, 0);
    printf("Réponse reçue : %s\n", messageRecu);

    // L'utilisateur entre son message
    char* messageEnvoie = (char*)malloc(TAILLE_MESS);
    fgets(messageEnvoie, TAILLE_MESS, stdin);

    send(dS, messageEnvoie, strlen(messageEnvoie) , 0);
    printf("Message Envoyé \n");
    if(strcmp(messageEnvoie, "fin") == 0){envoie = 0;}
  }

  shutdown(dS,2) ;
  printf("Fin du programme");
}