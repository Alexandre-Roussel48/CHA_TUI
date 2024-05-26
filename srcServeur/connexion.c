#include "headers/chat.h"

void createChat(int nbClients, int port, chat_args* args) {
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
	res.nb_clients = nbClients;

	// Init of users
	res.users = (user*)malloc(nbClients*sizeof(user));
	for (int i=0; i<nbClients; i++) {
		res.users[i].ad = -1;
	}

	// Init of sems
	int cle=ftok("1234", 'r');
	int idSem;

	if((idSem=semget(cle, 1, IPC_CREAT | IPC_EXCL | 0600)) == -1){
		perror("erreur semget : ");
		exit(-1);
	}

	ushort tabinit[1];
	tabinit[0] = nbClients;;

	union semun{
		int val;
		struct semid_ds * buf;
		ushort * array;
	} valinit;

	valinit.array = tabinit;

	if (semctl(idSem, 1, SETALL, valinit) == -1){
		perror("erreur initialisation sem : ");
		exit(1);
	}

	*args = res;

	printf("Chat created\n");
}

int emptyClient(int nbClients, user* users) {
	for (int i=0; i<nbClients; i++) {
		if (users[i].ad == -1) {
			return i;
		}
	}
	return -1;
}

int acceptUser(chat_args* args) {
	pthread_mutex_lock(&args->mutex_lock);
	int cleSem = ftok("1234", 'r');
	int idSem = semget(cleSem, 1, 0600);

	struct sembuf op[]={
		{(ushort)0,(short)-1,0},
		{(ushort)0,(short)+1,0},
		{(ushort)0, (short)0,0}};

	semop(idSem,op,1);
	pthread_mutex_unlock(&args->mutex_lock);

	chat_args res = *args;
	int index = emptyClient(res.nb_clients, res.users); // recupere l'index du nouveau client dans le tableau de clients

	if (index > -1) {
		socklen_t lg = sizeof(struct sockaddr_in);
		struct sockaddr_in aC;
		res.users[index].ad = accept(res.dS, (struct sockaddr*) &aC,&lg); // On connecte la connection avec le client et on stock l'adresse

		pthread_mutex_lock(&args->mutex_lock);
		*args = res;
		pthread_mutex_unlock(&args->mutex_lock);
		printf("Client %d connected\n", index);
		int startCode = 1;
		send(res.users[index].ad, &startCode, sizeof(int), 0);
		return index;
	}
	return -1;
}

void shutdownServer(chat_args* args) {
	for (int i=0; i<args->nb_clients; i++) {
		if (args->users[i].ad != -1) {
			shutdownClient(i, args->users, args->mutex_lock);
		}
	}
	shutdown(args->dS, 2);
	int cle=ftok("1234", 'r');
	int idSem = semget(cle, 1, 0600);
	semctl(idSem, 0, IPC_RMID, NULL);
	exit(0);
}

void shutdownClient(int index, user* users, pthread_mutex_t mutex_lock) {
	int shutdownCode = -1;
	send(users[index].ad, &shutdownCode, sizeof(int), 0);
	pthread_mutex_lock(&mutex_lock);
    shutdown(users[index].ad, 2);
    users[index].ad = -1;
    users[index].username = NULL;
	int cleSem = ftok("1234", 'r');
	int idSem = semget(cleSem, 1, 0600);
	pthread_mutex_unlock(&mutex_lock);

	struct sembuf op[]={
		{(ushort)0,(short)-1,0},
		{(ushort)0,(short)+1,0},
		{(ushort)0, (short)0,0}};

	semop(idSem,op+1,1);
}