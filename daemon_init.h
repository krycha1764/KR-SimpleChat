
#ifndef _DAEMON_INIT_H_
#define _DAEMON_INIT_H_

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <signal.h>

#include "signals.h"

int makeDaemon(const char *name, int sockfd);

#endif
