
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

#include "TLV.h"

#define SA struct sockaddr

int sockfd = 0;

size_t user_s = 0;
size_t pass_s = 0;
char* username = NULL;
char* password = NULL;

void* send_thread(void* arg);
void* recv_thread(void* arg);

void sig_sigpipe(int signo) {
	printf("Received SIGPIPE - exit\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(2137);
	int ret = 0;

	if(argc == 3) {
		servaddr.sin_addr.s_addr = inet_addr(argv[1]);
		servaddr.sin_port = htons(atoi(argv[2]));

	}else if(argc == 1) {
		size_t len;
		char *text = NULL;
		ret = 0;
		printf("SERVER ADDRESS: ");
		ret = getline(&text, &len, stdin);
		if(ret < 0) {
			printf("getline() ERROR %d : %s\n", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
		servaddr.sin_addr.s_addr = inet_addr(text);
		printf("SERVER PORT: ");
		ret = getline(&text, &len, stdin);
		if(ret < 0) {
			printf("getline() ERROR %d : %s\n", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
		servaddr.sin_port = htons(atoi(text));
		free(text);

	}else {
		printf("WRONG USAGE\n");
		exit(EXIT_FAILURE);
	}

	printf("username:\n");
	ret = getline(&username, &user_s, stdin);
	if(ret < 0) {
		printf("getline() ERROR %d : %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}else if(ret <= 5) {
		printf("username too short.\n");
		exit(EXIT_FAILURE);
	}
	printf("password:\n");
	ret = getline(&password, &pass_s, stdin);
	if(ret < 0) {
		printf("getline() ERROR %d : %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}else if(ret <= 5) {
		printf("password too short.\n");
		exit(EXIT_FAILURE);
	}
	for(size_t i = 0; i < user_s; i++) {
		if(username[i] == '\n') username[i] = '\0';
	}
	for(size_t i = 0; i < pass_s; i++) {
		if(password[i] == '\n') password[i] = '\0';
	}

	struct sigaction action;
	action.sa_handler = sig_sigpipe;
	sigemptyset (&action.sa_mask);
	action.sa_flags = 0;
	ret = sigaction(SIGPIPE, &action, NULL);
	if(ret < 0) {
		int err = errno;
		printf("sigaction() ERROR %d : %s\n", err, strerror(err));
		exit(EXIT_FAILURE);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if(sockfd <= 0) {
		int err = errno;
		printf("socket() ERROR %d: %s\n", err, strerror(err));
		exit(err);
	}

	ret = connect(sockfd, (SA *)&servaddr, sizeof(servaddr));

	if(ret != 0) {
		int err = errno;
		printf("connect() ERROR %d: %s\n", err, strerror(err));
		exit(err);
	}

	pthread_t sendt;
	pthread_t recvt;

	pthread_create(&sendt, NULL, send_thread, NULL);
	pthread_create(&recvt, NULL, recv_thread, NULL);

	pthread_join(sendt, NULL);
	pthread_join(recvt, NULL);

	exit(EXIT_SUCCESS);
}

void* send_thread(void* arg) {
	while(1) {
		size_t n = 0;
		char* text = NULL;
		int ret = 0;
		ret = getline(&text, &n, stdin);
		if(ret < 0) {
			printf("getline() ERROR %d : %s\n", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
		if(!strcmp(text, "QUIT\n")) {
			printf("QUITTING...\n");
			struct tlv msg;
			msg.length = sprintf(text, "TERM\n");
			msg.type = CONTROL;
			msg.data = (uint8_t*)text;
			send_tlv(sockfd, &msg);
			close(sockfd);
			exit(EXIT_SUCCESS);
			pthread_exit(NULL);
		}
		struct tlv msg;
		msg.type = MESSAGE;
		msg.length = strlen(text);
		msg.data = (uint8_t*)text;
		send_tlv(sockfd, &msg);
	}

	return NULL;
}

void* recv_thread(void* arg) {
	struct tlv receives;
	receives.data = NULL;
	struct tlv msg;
	msg.data = NULL;
	while (1) {
		recv_tlv(sockfd, &receives);
		char *text = NULL;
		switch (receives.type) {
			case MESSAGE:
				text = malloc(receives.length + 1);
				memset(text, 0, receives.length + 1);
				memcpy(text, receives.data, receives.length);
				printf("%s", text);
				free(text);
				text = NULL;
				break;
			case CONTROL:
				if(strcmp((char*)receives.data, "TERM\n")) {
					printf("QUITTING...\n");
					close(sockfd);
					exit(EXIT_SUCCESS);
				}else if(strcmp((char*)receives.data, "REJECT\n")) {
					printf("Server busy :(\n");
					close(sockfd);
					exit(EXIT_SUCCESS);
				}else if(strcmp((char*)receives.data, "SERV_ERROR\n")) {
					printf("Server error :(\n");
					close(sockfd);
					exit(EXIT_SUCCESS);
				}
				break;
			case NAME_REQ:
				msg.type = NAME;
				msg.data = (uint8_t*)username;
				msg.length = strlen(username);
				send_tlv(sockfd, &msg);
				break;
			case PASS_REQ:
				msg.type = PASS;
				msg.data = (uint8_t*)password;
				msg.length = strlen(password);
				send_tlv(sockfd, &msg);
				break;
			default:
			printf("WRONG DATA RECEIVED %X %X\n", MESSAGE, receives.type);
			exit(EXIT_FAILURE);
		}
		free(text);
		text = NULL;
		free(receives.data);
		receives.data = NULL;
	}
	return NULL;
}
