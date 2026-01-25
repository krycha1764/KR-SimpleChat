
#include "daemon_init.h"

int makeDaemon(const char *name, int sockfd) {
	pid_t pid;

	pid = fork();
	if(pid < 0) {
		exit(EXIT_FAILURE);
	}else if(pid > 0) {
		exit(EXIT_SUCCESS);
	}

	pid = setsid();
	if(pid < 0) {
		exit(EXIT_FAILURE);
	}

	setupSignalsHandlers(SH_SERVER);

	pid = fork();
	if(pid < 0) {
		exit(EXIT_FAILURE);
	}else if(pid > 0) {
		exit(EXIT_SUCCESS);
	}

	//chdir("/tmp");

	for(int i = sysconf(_SC_OPEN_MAX); i >= 0; i--) {
		if(i == sockfd) continue;
		close(i);
	}

	open("/dev/null", O_RDONLY);
	open("/dev/null", O_RDWR);
	open("/dev/null", O_RDWR);

	openlog(name, LOG_PID, LOG_USER);
	return 0;
}
