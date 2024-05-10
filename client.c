#include "srcClient/headers/chat.h"

int main(int argc, char *argv[]) {

  // Vérification des paramètres
  if(argc != 3){
    printf("Erreur: format de commande: ./client <ServeurIP> <PORT>\n");
    exit(EXIT_FAILURE);
  }

  chat_args args;
  createChat(&args, argv[1], atoi(argv[2]));

  if(launchChat(&args) == -1){
    shutdown(args.dS,2);
    printf("\x1b[34m");
    printf("Fin du chat\n");
    printf("\x1b[0m");
  }
}