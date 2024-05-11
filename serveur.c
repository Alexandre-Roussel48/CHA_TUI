#include "srcServeur/headers/chat.h"

chat_args args;

void sigint_handler(int sig) {
	shutdownServer(&args);
    exit(0);
}

int main(int argc, char *argv[]) {
	// Vérification des paramètres
	if(argc != 3){
		printf("Erreur: format de commande: ./serveur <NB_CLIENTS> <PORT>\n");
		exit(EXIT_FAILURE);
	}

	signal(SIGINT, sigint_handler);

	createChat(atoi(argv[1]), atoi(argv[2]), &args);

	while(1) {
		int index;
		do {
			if ((index = acceptUser(&args)) > -1) {launchChat(index, &args);}
		} while(index > -1);
	}
}