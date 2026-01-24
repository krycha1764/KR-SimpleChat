
#ifndef _TLV_H_
#define _TLV_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#define MAX_RECV_BUFF 20000

#define OLDRECV

enum types {
	INVALID,
	NAME_REQ,
	PASS_REQ,
	NAME,
	PASS,
	MESSAGE,
	CONTROL
};

struct tlv {
	uint8_t type;
	uint16_t length;
	uint8_t* data;
};

int send_tlv(int sockfd, struct tlv* tlvs);
int recv_tlv(int sockfd, struct tlv* tlvs);

int sendAll(int sockfd, uint8_t *buff, size_t len);
int readLength(int sockfd, uint8_t *buff, size_t length);

int sendMessage(int sockfd, int type, char* message);

#endif
