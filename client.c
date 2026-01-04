
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#define SA struct sockaddr

int main(int argc, char **argv) {

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(2137);

	if(argc == 3) {
		servaddr.sin_addr.s_addr = inet_addr(argv[1]);
		servaddr.sin_port = htons(atoi(argv[2]));

	}else if(argc == 1) {
		size_t len;
		char *text = NULL;
		printf("SERVER ADDRESS: ");
		getline(&text, &len, stdin);
		servaddr.sin_addr.s_addr = inet_addr(text);
		printf("SERVER PORT: ");
		getline(&text, &len, stdin);
		servaddr.sin_port = htons(atoi(text));
		free(text);

	}else {
		printf("WRONG USAGE\n");
		exit(EXIT_FAILURE);
	}


	int sockfd = 0;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if(sockfd <= 0) {
		int err = errno;
		printf("socket() ERROR %d: %s", err, strerror(err));
		exit(err);
	}

	int res = connect(sockfd, (SA *)&servaddr, sizeof(servaddr));

	if(res != 0) {
		int err = errno;
		printf("connect() ERROR %d: %s", err, strerror(err));
		exit(err);
	}
	sleep(10); // whole code here

	exit(EXIT_SUCCESS);
}
