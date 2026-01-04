
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/wait.h>

void sig_sigchild(int signo) {
	pid_t pid;
	int stat;

	while(1) {
		pid = waitpid(-1, &stat, WNOHANG);
		int err = errno;
		if(pid < 0) {
			printf("waitpid() ERROR %d : %s\n", err, strerror(err));
			exit(EXIT_FAILURE);
		}else if(pid > 0) {
			break;
		}
	}
}

void sig_sigpipe(int signo) {
	printf("Received SIGPIPE - exit\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {

	if(argc != 2) {
		 printf("USAGE:\n%s (PORT)\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int port = atoi(argv[1]);

	if(port < 0 || port > 65535) {
		printf("WRONG PORT NUMBER\n");
		exit(EXIT_FAILURE);
	}

	int sockfd = 0;
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	//servaddr.sin6_addr = in6addr_any;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if(sockfd <= 0) {
		int err = errno;
		printf("socket() ERROR: %s\n", strerror(err));
		exit(EXIT_FAILURE);
	}

	int ret = bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

	if(ret) {
		int err = errno;
		printf("bind() ERROR %d: %s\n", err, strerror(err));
		exit(EXIT_FAILURE);
	}

	ret = listen(sockfd, 10);

	if(ret) {
		int err = errno;
		printf("listen() ERROR %d: %s\n", err, strerror(err));
		exit(EXIT_FAILURE);
	}

	while(1) {
		struct sockaddr_in cliaddr;
		socklen_t clilen = sizeof(cliaddr);
		pid_t childpid;

		int connfd = accept(sockfd, (struct sockaddr*)&cliaddr, &clilen);
		if(connfd < 0) {
			int err = errno;
			printf("accept() ERROR %d : %s\n", err, strerror(err));
			exit(EXIT_FAILURE);
		}

		childpid = fork();
		int err = errno;
		if(childpid == 0) { // child
			close(sockfd);
			sleep(10); // client handling code here
			exit(EXIT_SUCCESS);

		}else if(childpid < 0) { // error
			printf("fork() ERROR %d : %s\n", err, strerror(err));
			exit(EXIT_FAILURE);

		}else if(childpid > 0) { // parent
			close(connfd);
		}
	}
	exit(EXIT_SUCCESS);
}
