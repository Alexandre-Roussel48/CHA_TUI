#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#define TAILLE_MESS 256
#define PORT 5001

int dS; // Initialisation du descripteur de la socket

// fonction qui permet d'arrêter le programme si un ctrl+C est exécuté
void sigint_handler(int signal) {
  // on force l'envoie du message fin pour terminer le programme
  char msg[140];
  strcpy(msg, "fin\n");
  int messageLength = strlen(msg);
  send(dS, &messageLength, sizeof(int), 0);
  send(dS, msg, messageLength*sizeof(char), 0);
  shutdown(dS, 2);
  exit(EXIT_SUCCESS);
}

void clear_last_line() {
    printf("\033[1A\033[2K"); // Move up one line, clear line
}

void* saisie(){
  char* messageEnvoie = (char*)malloc(TAILLE_MESS);
  while(1){
    printf("\t> ");

    // L'utilisateur 1 entre son message
    fgets(messageEnvoie, TAILLE_MESS, stdin);

    // Envoie de la taille du message au serveur
    int messageLength = strlen(messageEnvoie);
    send(dS, &messageLength, sizeof(int), 0);
    // Envoie du message au serveur
    send(dS, messageEnvoie, messageLength*sizeof(char), 0); 
    if(strcmp(messageEnvoie, "fin\n") == 0){break;} // Si le message est "fin" on arrete le programme
  }
  free(messageEnvoie);
  pthread_exit(0);
}

void* reception(){
  while(1){
    // L'utilisateur 1 recoit la taille du message
    int messageLength;
    if (recv(dS, &messageLength, sizeof(int), 0) > 0) {
      char* messageRecu = (char*)malloc(messageLength);
      // L'utilisateur 1 reçoit un message
      if (recv(dS, messageRecu, TAILLE_MESS*sizeof(char), 0) > 0) {
        printf("\033[s");
        printf("\033[1L");
        printf("\t/> %s", messageRecu);
        printf("\033[u");
        printf("\033[1B");
        fflush(stdout);
      }
      free(messageRecu);
    } else {
      break; // Permet de ne pas continuer la boucle si l'on ne reçoit plus aucun messages
    }
  }
  pthread_exit(0);
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
  if (connect(dS, (struct sockaddr *) &aS, lgA) < 0) { // Connection au serveur
    perror("Connection failed");
    exit(1);
  }
  printf("Socket Connecté\n");

  printf("\x1b[32m\n"); // Changement de couleur du texte pour la discussion

  pthread_t tsaisie;
  pthread_t treception;

  pthread_create(&tsaisie, NULL, saisie, 0);
  pthread_create(&treception, NULL, reception, 0);

  pthread_join(tsaisie, 0);
  pthread_join(treception, 0);

  shutdown(dS,2);
  printf("\x1b[34m");
  printf("Fin du chat\n");
  printf("\x1b[0m");
}