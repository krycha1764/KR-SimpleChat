
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

#include "TLV.h"
#include "signals.h"

#define MAX_CLIENTS 128

struct client_list {
	int clisock;
	pthread_t clithread;
	char* name;
	char* pass;
};

int histfd = 0;
pthread_mutex_t histmux = PTHREAD_MUTEX_INITIALIZER;
//struct client_list *clients = NULL;
struct client_list clients[MAX_CLIENTS];

void* client_handle(void *iter);
int client_getpass(struct client_list* client);
void client_free(struct client_list* client);

int main(int argc, char **argv) {

	if(argc != 2) {
		 printf("USAGE:\n%s (PORT)\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int port = atoi(argv[1]);
	int ret = 0;

	if(port < 0 || port > 65535) {
		printf("WRONG PORT NUMBER\n");
		exit(EXIT_FAILURE);
	}

	ret = setupSignalsHandlers(SH_SERVER);
	if(ret < 0) {
		printf("Error setting signal handlers %d : %s", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	//clients = malloc(MAX_CLIENTS * sizeof(struct client_list));
	memset(clients, 0, sizeof(struct client_list) * MAX_CLIENTS);

	int sockfd = 0;
	struct sockaddr_in6 servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin6_family = AF_INET6;
	servaddr.sin6_port = htons(port);
	servaddr.sin6_addr = in6addr_any;
	//servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	sockfd = socket(AF_INET6, SOCK_STREAM, 0);

	if(sockfd <= 0) {
		int err = errno;
		printf("socket() ERROR: %s\n", strerror(err));
		exit(EXIT_FAILURE);
	}

	ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

	if(ret < 0) {
		int err = errno;
		printf("setsockopt() ERROR %d: %s\n", err, strerror(err));
		exit(EXIT_FAILURE);
	}

	struct linger sl;
	sl.l_onoff = 1;
	sl.l_linger = 1;
	ret = setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &sl, sizeof(sl));

	if(ret < 0) {
		int err = errno;
		printf("setsockopt() ERROR %d: %s\n", err, strerror(err));
		exit(EXIT_FAILURE);
	}

	ret = bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

	if(ret) {
		int err = errno;
		printf("bind() ERROR %d: %s\n", err, strerror(err));
		exit(EXIT_FAILURE);
	}

	pthread_mutex_lock(&histmux);
	histfd = open("history", O_CREAT | O_RDWR, S_IRWXU);
	if(histfd < 0) {
		printf("open() ERROR %d: %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	pthread_mutex_unlock(&histmux);

	ret = listen(sockfd, 10);

	if(ret) {
		int err = errno;
		printf("listen() ERROR %d: %s\n", err, strerror(err));
		exit(EXIT_FAILURE);
	}

	while(1) {
		struct sockaddr_in cliaddr;
		socklen_t clilen = sizeof(cliaddr);

		int connfd = accept(sockfd, (struct sockaddr*)&cliaddr, &clilen);
		if(connfd < 0) {
			int err = errno;
			printf("accept() ERROR %d : %s\n", err, strerror(err));
			exit(EXIT_FAILURE);
		}

		size_t iterator = 0;
		int tooManyClients = 1;
		for(size_t i = 0; i < MAX_CLIENTS; i++) {
			if(clients[i].clisock != 0) {
				continue;
			}else if(clients[i].clisock == 0) {
				clients[i].clisock = connfd;
				iterator = i;
				tooManyClients = 0;
				break;
			}
		}
		if(tooManyClients) {
			sendMessage(sockfd, CONTROL, "REJECT\n");
			close(connfd);
			printf("Rejected client\n");
			continue;
		}
		printf("Client connected :)\n");

		ret = pthread_create(&clients[iterator].clithread, NULL, &client_handle, &iterator);
		if(ret != 0) {
			sendMessage(connfd, CONTROL, "SERV_ERROR\n");
			close(connfd);
		}

	}
	exit(EXIT_SUCCESS);
}

void* client_handle(void *iter) {
	size_t it = *(size_t*)iter;
	struct tlv receives;
	struct tlv msg;
	memset(&receives, 0, sizeof(receives));
	memset(&msg, 0, sizeof(msg));
	int ret = 0;
	ret = client_getpass(&clients[it]);
	if(ret != 0) {
		printf("client_getpass ERROR %d : %s", ret, strerror(ret));
		free(receives.data);
		client_free(&clients[it]);
		pthread_exit(NULL);
	}
	pthread_mutex_lock(&histmux);
	{
		char buff[1024] = {0};
		size_t size = 0;
		lseek(histfd, 0, SEEK_SET);
		while((size = read(histfd, buff, 1024))) {
			msg.type = MESSAGE;
			msg.length = size;
			msg.data = (uint8_t*)buff;
			ret = send_tlv(clients[it].clisock, &msg);
			if(ret < 0) {
				printf("send_tlv() ERROR %d : %s\n", errno, strerror(errno));
				close(clients[it].clisock);
				free(receives.data);
				client_free(&clients[it]);
				pthread_exit(NULL);
			}
		}
	}
	pthread_mutex_unlock(&histmux);
	while(1) {
		ret = recv_tlv(clients[it].clisock, &receives);
		if(ret < 0) {
			int err = errno;
			printf("recv_tlv() ERROR %d : %s\n", err, strerror(err));
			free(receives.data);
			client_free(&clients[it]);
			pthread_exit(NULL);
		}
		switch(receives.type) {
			case INVALID:
				continue;
				break;
			case MESSAGE:
				msg.data = malloc(strlen(receives.length + clients[it].name) + 5);
				msg.length = sprintf((char*)msg.data, "<%s>: %s", clients[it].name, receives.data);
				msg.type = MESSAGE;
				pthread_mutex_lock(&histmux);
				lseek(histfd, 0, SEEK_END);
				write(histfd, msg.data, strlen((char*)msg.data));
				pthread_mutex_unlock(&histmux);
				for(size_t i = 0; i < MAX_CLIENTS; i++) {
					if(clients[i].clisock == 0) continue;
					if(clients[i].clisock == clients[it].clisock) continue;
					ret = send_tlv(clients[i].clisock, &msg);
					//ret = send_tlv(clients[i].clisock, &receives);
					if(ret < 0) {
						int err = errno;
						printf("send_tlv() ERROR %d : %s\n", err, strerror(err));
						close(clients[it].clisock);
						free(receives.data);
						client_free(&clients[it]);
						pthread_exit(NULL);
					}
				}
				free(msg.data);
				break;
			case CONTROL:
				if(strcmp((char*)receives.data, "TERM\n") == 0) {
					printf("Client disconnected :(\n");
					//close(clients[it].clisock);
					free(receives.data);
					client_free(&clients[it]);
					pthread_exit(NULL);
				}
				break;
			default:
				printf("WRONG MESSAGE TYPE: %d", receives.type);
				close(clients[it].clisock);
				free(receives.data);
				client_free(&clients[it]);
				pthread_exit(NULL);
				break;
		}
		free(receives.data);
		receives.data = NULL;
	}
}

int client_getpass(struct client_list* client) {
	struct tlv msg;
	int ret = 0;

	msg.type = NAME_REQ;
	msg.length = 0;
	msg.data = NULL;
	ret = send_tlv(client->clisock, &msg);
	if(ret < 0) {
		return errno;
	}
	ret = recv_tlv(client->clisock, &msg);
	if(ret < 0) {
		return errno;
	}
	if(msg.type != NAME) return -1;
	client->name = realloc(client->name, strlen((char*)msg.data));
	strcpy(client->name, (char*)msg.data);
	free(msg.data);

	msg.type = PASS_REQ;
	msg.length = 0;
	msg.data = NULL;
	ret = send_tlv(client->clisock, &msg);
	if(ret < 0) {
		return errno;
	}
	ret = recv_tlv(client->clisock, &msg);
	if(ret < 0) {
		return errno;
	}
	if(msg.type != PASS) return -1;
	client->pass = realloc(client->pass, strlen((char*)msg.data));
	strcpy(client->pass, (char*)msg.data);
	free(msg.data);
	return 0;
}

void client_free(struct client_list* client) {
	if(client->clisock) close(client->clisock);
	client->clisock = 0;
	client->clithread = 0;
	if(client->name) free(client->name);
	if(client->pass) free(client->pass);
	client->name = NULL;
	client->pass = NULL;
}
