#include "serveur.h"

#define PORT 5001

int dS;
int NB_CLIENTS;   // Definie le nombre de clients
User *users;      // tableau d'utilisateurs
pthread_mutex_t mutex_lock;

void init() {
  for(int i=0; i<NB_CLIENTS; i++){
    users[i].ad = -1; // Initialisation du tableau d'adresse
  }

  printf("\x1b[32m");// Permet de mettre le texte en couleur
  printf("Début programme\n");
  dS = socket(PF_INET, SOCK_STREAM, 0); // Initialisation du descripteur de la socket
  printf("Socket Créé\n");

  struct sockaddr_in ad;
  ad.sin_family = AF_INET; // L'IP du serveur sera une IPv4
  ad.sin_addr.s_addr = INADDR_ANY; // Permet d'écouter toutes les adresses
  ad.sin_port = htons(PORT); // Permet de spécifier le port sûr lequel se connecter sous forme binaire

  int optval = 1;
  setsockopt(dS, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)); // Permet de réutiliser un socket
  
  if (bind(dS, (struct sockaddr*)&ad, sizeof(ad)) < 0) {
    perror("Bind failed");
    exit(EXIT_FAILURE);
  }
  printf("Socket Nommé\n");


  listen(dS, 2) ;
  printf("Mode écoute\n\n");
  printf("\x1b[0m"); // Permet de changer la couleur du texte
}

/**
 * Regarde la premier place libre dans le tableau de Users
 * @return {int} index de la place libre dans le tableau
*/
int empty_client() {
  for (int i=0; i<NB_CLIENTS; i++) {
    if (users[i].ad == -1) {
      return i;
    }
  }
  return -1;
}

/**
 * Envoie le message et la taille du message à tous les utilisateurs
 * @params{int, int, char*}
 *    - index de l'utilisateur dans le tableau (int)
 *    - taille du message (int)
 *    - message (char*)
*/
void broadcast(int index, int messageLength, char* msg) {
  for (int i=0; i < NB_CLIENTS; i++) {
    if (i!=index && users[i].ad != -1) {
      send(users[i].ad, &messageLength, sizeof(int), 0);
      send(users[i].ad, msg, messageLength*sizeof(char), 0);
    }
  }
}

/**
 * transmet un message recu vers un client
 * @param{void* t} correspond à l'index du client
*/
void* transmission(void *t) {
  int user_index = (long)t;
  while(1) {
    // Reçoie de la taille du message
    int messageLength;
    if (recv(users[user_index].ad, &messageLength, sizeof(int), 0) > 0) {
      printf("Taille : %d\n", messageLength);
      char* msg = calloc(messageLength, sizeof(char));
      // Reçoie un message de l'envoyeur
      if (recv(users[user_index].ad, msg, messageLength*sizeof(char), 0) > 0) {
        printf("Message reçu Client %d: %s\n",user_index, msg);

        if (strcmp(msg, "fin\n") == 0) {
          break;
        }

        // Transmission du message vers le receveur
        broadcast(user_index, messageLength, msg);
      }
      free(msg);
    } else {
      break;
    }
  }
  shutdown((long)users[user_index].ad, 2);
  pthread_mutex_lock(&mutex_lock);
  users[user_index].ad = -1;
  pthread_mutex_unlock(&mutex_lock);
  printf("Client %d disconnected\n", user_index);
  pthread_exit(0);
}

/**
 * permet de recevoir la taille d'un message avec gestion d'erreur
 * @return {int} renvoie -1 si il y a une erreur lors de la reception
 *               sinon renvoie la taille du message
*/
int recv_message_length(int adresseClient){
  int messageLength;
  if(recv(adresseClient, &messageLength, sizeof(int), 0) <= 0){
    return -1;
  }
  return messageLength;
}

/**
 * permet de recevoir un message
 * @params {int adresseClient, int messageLength}
 * @return {MessageResult*} qui contient un errorCode et le message
*/
MessageResult* recv_message(int adresseClient, int messageLength){
  MessageResult* message = malloc(sizeof(MessageResult));
  if(message == NULL){
    message->errorCode = -1;
    message->message = "Erreur d'allocation memoire du type MessageResult"; // Pourquoi pas besoin d'allocation
    return message;
  }

  message->message = malloc(messageLength*sizeof(char));

  if (recv(adresseClient, message->message, messageLength*sizeof(char), 0) <= 0){
    message->errorCode = -1;
    message->message = "Erreur lors de la recption du message";
    return message;
  }

  message->errorCode = 1;
  return message;
}

/**
 * permet de connecter un client en lui demandant son username
*/
void connect_users() {
  while(1) {
    int index = empty_client(); // recupere l'index du nouveau client dans le tableau de clients

    while(index > -1) { // ?

      printf("\x1b[34m");

      socklen_t lg = sizeof(struct sockaddr_in) ;
      struct sockaddr_in aC;
      users[index].ad = accept(dS, (struct sockaddr*) &aC,&lg); // On connecte la connection avec le client et on stock l'adresse

      int messageLength;
      messageLength = recv_message_length(users[index].ad);
      if(messageLength == -1){
        perror("erreur taille message");
        exit(EXIT_FAILURE);
      }
      printf("Taille : %d\n", messageLength);

      MessageResult* messageResult = recv_message(users[index].ad, messageLength);
      if(messageResult->errorCode == -1){
        printf("%s", messageResult->message);
        exit(EXIT_FAILURE);
      }
      char* message = messageResult->message;
      printf("Message reçu Client %d: %s\n",index, message);

      users[index].username = message;

      printf("%s Connecté\n", users[index].username);

      pthread_create(&users[index].thread, 0, transmission, (void*)(long)index);

      index = empty_client();

      free(messageResult->message);
      free(messageResult);
    }
  }
}

int main(int argc, char *argv[]) {
  // Vérification des paramètres
  if(argc != 2){
    printf("Erreur: format de commande: ./serveur <NB_CLIENTS>\n");
    exit(EXIT_FAILURE);
  }
  NB_CLIENTS = atoi(argv[1]);
  users = (User *)malloc(NB_CLIENTS * sizeof(User));

  init();
  connect_users();
}