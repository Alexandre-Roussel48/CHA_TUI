#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define TAILLE_MESS 140
#define PORT 5001

int dS; // Initialisation du descripteur de la socket

// fonction qui permet d'arrêter le programme si un ctrl+C est exécuté
void sigint_handler(int signal) {
  // on force l'envoie du message fin pour terminer le programme
  char msg[140];
  strcpy(msg, "fin\n");
  send(dS, msg, TAILLE_MESS*sizeof(char), 0);
  shutdown(dS, 2);
  exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {

  // Vérification des paramètres
  if(argc != 2){
    printf("Erreur: format de commande: ./client1 <ServeurIP>");
    exit(EXIT_FAILURE);
  }

  printf("\x1b[34m"); // Permet de mettre le texte en couleur
  dS = socket(PF_INET, SOCK_STREAM, 0); // Création du socket pour le protocole TCP
  printf("Socket Créé\n");

  signal(SIGINT, sigint_handler); // Gestion du Ctrl+C

  struct sockaddr_in aS;
  aS.sin_family = AF_INET; // L'IP du serveur sera une IPv4
  inet_pton(AF_INET,argv[1],&(aS.sin_addr)) ; // Permet de spécifier l'adresse du serveur sous forme binaire
  aS.sin_port = htons(PORT) ; // Permet de spécifier le port sûr lequel se connecter sous forme binaire
  socklen_t lgA = sizeof(struct sockaddr_in);

  connect(dS, (struct sockaddr *) &aS, lgA); // Connection au serveur
  printf("Socket Connecté\n");

  printf("\x1b[32m\n"); // Changement de couleur du texte pour la discussion
  while(1){
    printf("\t> ");
    // L'utilisateur 1 entre son message
    char* messageEnvoie = (char*)malloc(TAILLE_MESS);
    fgets(messageEnvoie, TAILLE_MESS, stdin);

    // Envoie du message au serveur
    send(dS, messageEnvoie, TAILLE_MESS*sizeof(char), 0); 
    if(strcmp(messageEnvoie, "fin\n") == 0){break;} // Si le message est "fin" on arrete le programme

    // L'utilisateur 1 reçoit un message
    char* messageRecu = (char*)malloc(TAILLE_MESS);
    if (recv(dS, messageRecu, TAILLE_MESS*sizeof(char), 0) == 0) {
      break; // Permet de ne pas continuer la boucle si l'on ne reçoit plus aucun messages
    }
    printf("\t/> %s", messageRecu);
  }

  shutdown(dS,2);
  printf("\x1b[34m");
  printf("Fin du chat\n");
  printf("\x1b[0m");
}