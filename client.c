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
 * gestion des commandes d'affichage
*/
int find_first_slash(char *str) {
    if (str == NULL)
        return -1;

    int index = 0;
    while (*str != '\0') {
        if (*str == '/') {
            return index;
        }
        str++;
        index++;
    }

    // If '/' is not found, return -1
    return -1;
}

/**
 * fonction qui permet d'envoyer : taille message + message
*/
int send_message(char* message) {
  int messageLength = strlen(message)+1;
  if (send(dS, &messageLength, sizeof(int), 0) < 0) {
    perror("messageLength send failed\n");
    return -1;
  }
  if (send(dS, message, messageLength*sizeof(char), 0) < 0) {
    perror("message send failed\n");
    return -1;
  }
  return 1;
}

int recv_message() {
  int messageLength;
  int res = recv(dS, &messageLength, sizeof(int), 0); // reception taille message
  if (res < 0) { // gestion erreur
    perror("messageLength receive failed\n");
    return -1;
  } else if (res == 0) {
    return 0;
  }
  char* messageRecu = (char*)malloc(messageLength);
  res = recv(dS, messageRecu, TAILLE_MESS*sizeof(char), 0);
  if (res < 0) {
    free(messageRecu);
    perror("message receive failed\n");
    return -1;
  } else if (res == 0) {
    free(messageRecu);
    return 0;
  }
  printf("\033[s\033[1L\t/> %s\033[u\033[1B", messageRecu); // affichage plus joli
  fflush(stdout); // force la maj de la sortie standard
  free(messageRecu);
  return 1;
}

/**
 * permet au client de saisir un message
 * est utilisé dans un thread
 * @return {void*}
*/
void* saisie(){
  int res = 1;
  while(res == 1){
    char* messageEnvoie = (char*)malloc(TAILLE_MESS);
    printf("\t> ");

    // L'utilisateur 1 entre son message
    fgets(messageEnvoie, TAILLE_MESS, stdin);

    /*int command_index = find_first_slash(messageEnvoie);

    if (command_index != -1) {
      if(strcmp(messageEnvoie+command_index, "/tableflip\n")==0) {
        strcpy(messageEnvoie+command_index, "(╯°□ °)╯︵ ┻━┻\n");
      }
      else if (strcmp(messageEnvoie+command_index, "/unflip\n")==0) {
        strcpy(messageEnvoie+command_index, "┬─┬ノ( º _ ºノ)\n");
      }
      else if (strcmp(messageEnvoie+command_index, "/shrug\n")==0) {
        strcpy(messageEnvoie+command_index, "¯\\_(ツ)_/¯\n");
      }
    }*/

    // Envoie de la taille du message et du message au serveur
    res = send_message(messageEnvoie);

    free(messageEnvoie);
  }
  pthread_exit(0);
}

/**
 * permet au client de lire un message recu
 * est utilisé dans un thread
 * @return {void*}
*/
void* reception(){
  int res = 1;
  while(res == 1){
    // L'utilisateur 1 recoit la taille du message
    res = recv_message();
  }
  pthread_exit(0);
}

/**
 * permet de demander le username du client
*/
int ask_username() {
  char* messageEnvoie = (char*)malloc(TAILLE_MESS);
  printf("\t> Veuillez entrer votre Username :\n");
  printf("\t> ");
  // L'utilisateur entre son username
  fgets(messageEnvoie, TAILLE_MESS, stdin);
  if (send_message(messageEnvoie) < 0) {
    perror("Username binding failed\n");
    return -1;
  }
  return 1;
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

int main(int argc, char *argv[]) {

  // Vérification des paramètres
  if(argc != 2){
    printf("Erreur: format de commande: ./client <ServeurIP>\n");
    exit(EXIT_FAILURE);
  }

  init(argv[1]);
  if (ask_username() < 0) {goto shutdown;}

  printf("\x1b[32m\n"); // Changement de couleur du texte pour la discussion

  pthread_t tsaisie;
  pthread_t treception;

  pthread_create(&tsaisie, NULL, saisie, 0); // Envoie d'un message
  pthread_create(&treception, NULL, reception, 0); // Reception d'un message

  pthread_join(tsaisie, 0);
  pthread_join(treception, 0);

  shutdown:
  shutdown(dS,2);
  printf("\x1b[34m");
  printf("Fin du chat\n");
  printf("\x1b[0m");
}