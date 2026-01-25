
#ifndef _SIGNALS_H_
#define _SIGNALS_H_

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

enum sigHandlers {
	SH_CLIENT,
	SH_SERVER
};

int setupSignalsHandlers(int app);

void sig_sigchild(int signo);
void sig_sighup(int signo);
void sig_sigpipeSERVER(int signo);
void sig_sigpipeCLIENT(int signo);

#endif // _SIGNALS_H_
