
#include "TLV.h"

int send_tlv(int sockfd, struct tlv* tlvs) {
	size_t buff_size = sizeof(tlvs->length) + sizeof(tlvs->type) + tlvs->length;
	uint8_t* buff = malloc(buff_size);
	memcpy((buff), &(tlvs->type), sizeof(tlvs->type));
	memcpy((buff + sizeof(tlvs->type)), &(tlvs->length), sizeof(tlvs->length));
	memcpy((buff + sizeof(tlvs->type) + sizeof(tlvs->length)), (tlvs->data), tlvs->length);
	int ret = sendAll(sockfd, buff, buff_size);
	if(ret < 0) return ret;
	free(buff);
	return ret;
}

#ifdef OLDRECV

int recv_tlv(int sockfd, struct tlv* tlvs) {
	uint8_t* buff = malloc(MAX_RECV_BUFF);
	int received = recv(sockfd, buff, MAX_RECV_BUFF, 0);
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

#else

int recv_tlv(int sockfd, struct tlv* tlvs) {
	uint8_t *buff = malloc(sizeof(tlvs->length) + sizeof(tlvs->type));
	int ret = readLength(sockfd, buff, sizeof(tlvs->length) + sizeof(tlvs->type));
	if(ret < 0) {
		free(buff);
		tlvs->type = INVALID;
		return -1;
	}
	memcpy(&(tlvs->type), (buff), sizeof(tlvs->type));
	memcpy(&(tlvs->length), (buff + sizeof(tlvs->type)), sizeof(tlvs->length));
	if(tlvs->length == 0) return 0;
	tlvs->data = malloc(tlvs->length);
	free(buff);
	ret = readLength(sockfd, tlvs->data, tlvs->length);
	if(ret < 0) {
		free(tlvs->data);
		tlvs->type = INVALID;
		return -1;
	}
	return 0;
}

#endif

int sendAll(int sockfd, uint8_t *buff, size_t len) {
	size_t sended = 0;
	size_t tosend = len;
	int ret;
	while(sended < len) {
		ret = write(sockfd, buff + sended, tosend);
		if(ret == -1) return -1;
		sended += ret;
		tosend -= ret;
	}
	return 0;
}

int readLength(int sockfd, uint8_t *buff, size_t length) {
	size_t received = 0;
	size_t torecv = length;
	int ret = 0;
	while(torecv > 0) {
		ret = read(sockfd, buff + received, torecv);
		if(ret == -1) return -1;
		received += ret;
		torecv -= ret;
	}
	return received;
}

int sendMessage(int sockfd, int type, char* message) {
	struct tlv msg;

	msg.type = type;
	msg.length = strlen(message);
	msg.data = (uint8_t*)message;
	return send_tlv(sockfd, &msg);
}
