
#include "users.h"

char **passwords = NULL;
char **usernames = NULL;
size_t users = 0;

int readListFile(int fd) {
	size_t listsize = 0;
	char *tmpline = NULL;
	size_t tmpline_s = 0;

	lseek(fd, 0, SEEK_SET);
	FILE *mfile = fdopen(fd, "w+");
	while(1) {
		size_t read = getline(&tmpline, &tmpline_s, mfile);
		if(read == -1) {
			if(errno == 0) break;
			return -1;
		}
		usernames = realloc(usernames, ++listsize);
		passwords = realloc(passwords, listsize);
		usernames[listsize - 1] = strtok(tmpline, "\t");
		passwords[listsize - 1] = strtok(NULL, "\n");
	}
	users = listsize;
	return 0;
}

int addUser(int fd, char *username, char *password) {
	int ret = 0;
	passwords = realloc(passwords, ++users);
	usernames = realloc(usernames, users);
	passwords[users - 1] = malloc(sizeof(password) + 1);
	usernames[users - 1] = malloc(sizeof(username) + 1);
	strcpy(passwords[users - 1], password);
	strcpy(usernames[users - 1], username);
	lseek(fd, 0, SEEK_END);
	char *text = malloc(strlen(username) + strlen(password) + 5);
	size_t text_s = sprintf(text, "%s\t%s\n", username, password);
	ret = write(fd, text, text_s);
	free(text);
	if(ret < 0) return -1;
	return 0;
}

int verfyUser(char *username, char *password) {
	int res = 0;
	for(size_t i = 0; i < users; i++) {
		if(strcmp(username, usernames[i]) != 0) continue;
		if(strcmp(password, passwords[i]) != 0) continue;
		res = 1;
		break;
	}
	return res;
}

