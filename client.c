#include "client.h"

int dS; // Initialisation du descripteur de la socket

/**
 * fonction qui permet d'arrêter le programme si un ctrl+C est exécuté
 * @param {int}
*/
void sigint_handler(int signal) {
  char msg[140];
  strcpy(msg, "fin\n"); // on force l'envoie du message fin pour terminer le programme
  
  int messageLength = strlen(msg); // envoie de la taille et du message
  send(dS, &messageLength, sizeof(int), 0);
  send(dS, msg, messageLength*sizeof(char), 0);

  shutdown(dS, 2);
  exit(EXIT_SUCCESS);
}

/**
 * permet la création de socket et la connexion au serveur
 * @params {char*} correspond à l'address du serveur
*/
void init(char* address) {
  printf("\x1b[34m"); // Permet de mettre le texte en couleur
  
  dS = socket(PF_INET, SOCK_STREAM, 0); // Création du socket pour le protocole TCP
  printf("Socket Créé\n");

  signal(SIGINT, sigint_handler); // Gestion du Ctrl+C

  struct sockaddr_in aS;
  aS.sin_family = AF_INET; // L'IP du serveur sera une IPv4
  inet_pton(AF_INET,address,&(aS.sin_addr)) ; // Permet de spécifier l'adresse du serveur sous forme binaire
  aS.sin_port = htons(PORT) ; // Permet de spécifier le port sûr lequel se connecter sous forme binaire
  socklen_t lgA = sizeof(struct sockaddr_in);

  if (connect(dS, (struct sockaddr *) &aS, lgA) < 0) { // test de la connexion au serveur
    perror("Connection failed");
    exit(EXIT_FAILURE);
  }
  printf("Socket Connecté\n");
}

/**
 * permet au client de saisir un message
 * est utilisé dans un thread
 * @return {void*}
*/
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

/**
 * permet au client de lire un message recu
 * est utilisé dans un thread
 * @return {void*}
*/
void* reception(){
  while(1){
    // L'utilisateur 1 recoit la taille du message
    int messageLength;
    if (recv(dS, &messageLength, sizeof(int), 0) > 0) {
      char* messageRecu = (char*)malloc(messageLength);
      // L'utilisateur 1 reçoit un message
      if (recv(dS, messageRecu, TAILLE_MESS*sizeof(char), 0) > 0) {
        printf("\033[s"); // permet un affichage plus joli
        printf("\033[1L");
        printf("\t/> %s", messageRecu);
        printf("\033[u");
        printf("\033[1B");
        fflush(stdout); // force la maj de la sortie standard
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

  init(argv[1]);

  printf("\x1b[32m\n"); // Changement de couleur du texte pour la discussion

  pthread_t tsaisie;
  pthread_t treception;

  pthread_create(&tsaisie, NULL, saisie, 0); // Envoie d'un message
  pthread_create(&treception, NULL, reception, 0); // Reception d'un message

  pthread_join(tsaisie, 0);
  pthread_join(treception, 0);

  shutdown(dS,2);
  printf("\x1b[34m");
  printf("Fin du chat\n");
  printf("\x1b[0m");
}