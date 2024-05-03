#include "srcServeur/headers/chat.h"

int main(int argc, char *argv[]) {
	// Vérification des paramètres
	if(argc != 3){
		printf("Erreur: format de commande: ./serveur <NB_CLIENTS> <PORT>\n");
		exit(EXIT_FAILURE);
	}

	chat_args args;
	createChat(atoi(argv[1]), atoi(argv[2]), &args);

	while(1) {
		int index;
		do {
			if ((index = acceptUser(&args)) > -1) {launchChat(index, &args);}
		} while(index > -1);
	}
}