
#include "signals.h"

int setupSignalsHandlers(int app) {
	struct sigaction action;
	action.sa_handler = sig_sigchild;
	sigemptyset(&action.sa_mask);
	action.sa_flags = SA_RESTART;
	int ret = sigaction(SIGCHLD, &action, NULL);
	if(ret < 0) {
		return ret;
	}

	if(app == SH_SERVER) {
		action.sa_handler = sig_sigpipeSERVER;
	} else if(app == SH_SERVER) {
		action.sa_handler = sig_sigpipeCLIENT;
	}
	sigemptyset(&action.sa_mask);
	action.sa_flags = SA_RESTART;
	ret = sigaction(SIGPIPE, &action, NULL);
	if(ret < 0) {
		return ret;
	}

	return 0;
}

void sig_sigchild(int signo) {
	int status;
	while(waitpid(-1, &status, WNOHANG) > 0);
}

void sig_sigpipeSERVER(int signo) {
	printf("Received SIGPIPE\n");
}

void sig_sigpipeCLIENT(int signo) {
	printf("Received SIGPIPE, EXIT\n");
	exit(EXIT_FAILURE);
}
