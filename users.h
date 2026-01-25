
#ifndef _USERS_H_
#define _USERS_H_

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

int readListFile(int fd);
int addUser(int fd, char *username, char *password);
int verfyUser(char *username, char *password);

#endif
