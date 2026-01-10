
#include "TLV.h"
#include <stdlib.h>
#include <string.h>

int send_tlv(int sockfd, struct tlv* tlvs) {
	size_t buff_size = sizeof(tlvs->length) + sizeof(tlvs->type) + tlvs->length;
	uint8_t* buff = malloc(buff_size);
	memcpy((buff), &(tlvs->type), sizeof(tlvs->type));
	memcpy((buff + sizeof(tlvs->type)), &(tlvs->length), sizeof(tlvs->length));
	memcpy((buff + sizeof(tlvs->type) + sizeof(tlvs->length)), (tlvs->data), tlvs->length);
	int ret = send(sockfd, buff, buff_size, 0);
	if(ret < 0) return ret;
	free(buff);
	return ret;
}

int recv_tlv(int sockfd, struct tlv* tlvs) {
	uint8_t* buff = malloc(MAX_RECV_BUFF);
	size_t received = recv(sockfd, buff, MAX_RECV_BUFF, 0);
	if(received < (sizeof(tlvs->type) + sizeof(tlvs->length))) {
		free(buff);
		tlvs->type = INVALID;
		return -1;
	}
	memcpy(&(tlvs->type), (buff), sizeof(tlvs->type));
	memcpy(&(tlvs->length), (buff + sizeof(tlvs->type)), sizeof(tlvs->length));
	tlvs->data = malloc(tlvs->length);
	memcpy((tlvs->data), (buff + sizeof(tlvs->type) + sizeof(tlvs->length)), tlvs->length);
	free(buff);
	return 0;
}
