#include "headers/chat.h"

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
		return 0;
	}
	return 1;
}

int sendUsername(int index, user* users, char* username) {
	int usernameLength = strlen(username) + 1;
	if (send(users[index].ad, &usernameLength, sizeof(int), 0) < 0) {return -1;}
	if (send(users[index].ad, username, usernameLength*sizeof(char), 0) < 0) {return -1;}
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
			sendUsername(i, args->users, args->users[index].username);
			sendMsg(i, msg, msgLength, args->users);
		}
	}
	free(msg);
}

void* transmission(void *args) {
	typedef struct {int index; chat_args chat;} trans_args;

	trans_args* t = (trans_args*)args;

	int msgLength;
	char* msg;
	do {
		if ((msgLength = recvMsgLength(t->index, t->chat.users)) <= 0) {
			shutdownClient(t->index, t->chat.users, t->chat.mutex_lock);
			break;
		}
		int res;
		if ((res = recvMsg(t->index, msgLength, &msg, t->chat.users)) < 0) {
			shutdownClient(t->index, t->chat.users, t->chat.mutex_lock);
			break;
		}
		if (res > 0) {
			int command;
			if ((command = checkCommand(msg, t->chat.mutex_lock)) < 0) {broadcast(t->index, msg, msgLength, &t->chat);}
			else if (command == 0) {commands(t->index, t->chat.users);}
			else if (command == 1) {members(t->index, t->chat.nb_clients, t->chat.users);}
			else if (command == 2) {whisper(t->index, msg, msgLength, t->chat.nb_clients, t->chat.users, t->chat.mutex_lock);}
			else if (command == 3) {kick(t->index, msg, msgLength, t->chat.nb_clients, t->chat.users, t->chat.mutex_lock);}
			else if (command == 4) {shutdownClient(t->index, t->chat.users, t->chat.mutex_lock);}
			else if (command == 5) {recvFile(t->index, msg, msgLength, t->chat.users, t->chat.mutex_lock);}
			else if (command == 6) {listFiles(t->index, t->chat.users);}
			else if (command == 7) {sendFile(t->index, msg, msgLength, t->chat.users, t->chat.mutex_lock);}
		}
	} while(1);

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