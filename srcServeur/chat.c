#include "headers/chat.h"

void createChat(int nb_clients, int port, chat_args* args) {
	chat_args res;

	// Init of the socket
	res.dS = socket(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in ad;
	ad.sin_family = AF_INET; // L'IP du serveur sera une IPv4
	ad.sin_addr.s_addr = INADDR_ANY; // Permet d'écouter toutes les adresses
	ad.sin_port = htons(port); // Permet de spécifier le port sûr lequel se connecter sous forme binaire

	int optval = 1;
	setsockopt(res.dS, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)); // Permet de réutiliser un socket

	if (bind(res.dS, (struct sockaddr*)&ad, sizeof(ad)) < 0) {
		perror("Bind failed");
		exit(EXIT_FAILURE);
	}

	listen(res.dS, 2);

	// Init of the number of clients
	res.nb_clients = nb_clients;

	// Init of users
	res.users = (user*)malloc(nb_clients*sizeof(user));
	for (int i=0; i<nb_clients; i++) {
		res.users[i].ad = -1;
	}

	*args = res;

	printf("Chat created\n");
}

int empty_client(int nb_clients, user* users) {
	for (int i=0; i<nb_clients; i++) {
		if (users[i].ad == -1) {
			return i;
		}
	}
	return -1;
}

int acceptUser(chat_args* args) {
	chat_args res = *args;
	int index = empty_client(res.nb_clients, res.users); // recupere l'index du nouveau client dans le tableau de clients

	if (index > -1) {
		socklen_t lg = sizeof(struct sockaddr_in);
		struct sockaddr_in aC;
		res.users[index].ad = accept(res.dS, (struct sockaddr*) &aC,&lg); // On connecte la connection avec le client et on stock l'adresse

		pthread_mutex_lock(&args->mutex_lock);
		*args = res;
		pthread_mutex_unlock(&args->mutex_lock);
		printf("Client %d connected\n", index);
		return index;
	}
	return -1;
}

int recvMsgLength(int index, user* users) {
	int msgLength;
	if (recv(users[index].ad, &msgLength, sizeof(int), 0) <= 0) {return -1;}
	return msgLength;
}

int recvMsg(int index, int msgLength, char** msg, user* users) {
	char* msgRecv = (char*)malloc(msgLength);
	if (recv(users[index].ad, msgRecv, msgLength*sizeof(char), 0) <= 0) {
		free(msgRecv);
		return -1;
	}

	*msg = msgRecv;
	if (users[index].username==NULL) {
		printf("Client %d is named %s", index, msgRecv);
		users[index].username = msgRecv;
		return 2;
	}
	printf("  $ %s  \x1b[34m%s\x1b[0m", users[index].username, msgRecv);
	return 1;
}

int sendMsg(int index, char* msg, int msgLength, user* users) {
	if (send(users[index].ad, &msgLength, sizeof(int), 0) < 0) {return -1;}
	if (send(users[index].ad, msg, msgLength*sizeof(char), 0) < 0) {return -1;}
	return 1;
}

void broadcast(int index, char* msg, int msgLength, chat_args* args) {
	for (int i=0; i < args->nb_clients; i++) {
		if (i != index && args->users[i].ad != -1) {
			sendMsg(i, msg, msgLength, args->users);
		}
	}
}

void* transmission(void *args) {
	typedef struct {int index; chat_args chat;} trans_args;

	trans_args* t = (trans_args*)args;

	int msgLength;
	char* msg;
	do {
		if ((msgLength = recvMsgLength(t->index, t->chat.users)) <= 0) {break;}
		int res;
		if ((res = recvMsg(t->index, msgLength, &msg, t->chat.users)) <= 0) {break;}
		if (res == 1) {broadcast(t->index, msg, msgLength, &t->chat);}
	} while(strcmp(msg, "fin\n") != 0);

	shutdown((long)t->chat.users[t->index].ad, 2);
	pthread_mutex_lock(&t->chat.mutex_lock);
	t->chat.users[t->index].ad = -1;
	pthread_mutex_unlock(&t->chat.mutex_lock);
	printf("Client %d disconnected\n", t->index);
	pthread_exit(0);
}


void launchChat(int index, chat_args* args) {
	typedef struct {int index; chat_args chat;} trans_args;

	trans_args* t = (trans_args*)malloc(sizeof(trans_args));
	t->index = index;
	t->chat = *args;
	pthread_create(&args->users[index].thread, 0, transmission, (void*)t);
}