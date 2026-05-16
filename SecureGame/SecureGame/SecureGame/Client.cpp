#include "Client.h"

// =======================================================
// Constructor / Destructor
// =======================================================



Client::Client(const char* ip, const unsigned short u_port)
{
	if (WSAStartup(MAKEWORD(2, 2), &this->wsaData) != 0) {
		fputs("WSAStartup() error\n", stderr);
		exit(1);
	}

	this->sock = socket(AF_INET, SOCK_STREAM, 0);
	if (this->sock == INVALID_SOCKET) {
		fputs("socket() error\n", stderr);
		exit(1);
	}

	memset(&this->clntAddr, 0, sizeof(this->clntAddr));
	this->clntAddr.sin_family = AF_INET;
	this->clntAddr.sin_port = htons(u_port);
	this->clntAddr.sin_addr.S_un.S_addr = inet_addr(ip);

	this->connected = false;
	this->status = 0;
	this->clnt_number = 0;
}

Client::~Client() {
	closesocket(this->sock);
	WSACleanup();
}


// =======================================================
// Connect
// =======================================================

int Client::connectToServer() {
	if (connect(this->sock, (sockaddr*)&this->clntAddr, sizeof(this->clntAddr)) == SOCKET_ERROR) {
		fputs("connect() error\n", stderr);
		return 0;
	}
	connected = true;
	return 1;
}


// =======================================================
// send_all / recv_all
// =======================================================

int Client::send_all(const void* data, size_t size)
{
	size_t sent_total = 0;
	const unsigned char* p = (const unsigned char*)data; // (1) this is fine, because trying to read the data 

	while (sent_total < size) {
		int sent = send(this->sock, (const char*)p + sent_total, (int)(size - sent_total), 0);
		if (sent <= 0) return 0;
		sent_total += sent;
	}
	return 1;
}

int Client::recv_all(void* data, size_t size)
{
	size_t recv_total = 0;
	unsigned char* p = (unsigned char*)data; // (2) was trying to write to const unsigned char* which cus error

	while (recv_total < size) {
		int recved = recv(this->sock, (char*)p + recv_total, (int)(size - recv_total), 0);
		if (recved <= 0) return 0;
		recv_total += recved;
	}
	return 1;
}

int Client::send_packet(const void* buffer, uint16_t buffer_len) {
	uint16_t net_sent = htons(buffer_len);
	if (!send_all(&net_sent, sizeof(uint16_t))) {
		fputs("send_packet() error size", stdout);
		return 0;
	}

	if (!send_all(buffer, buffer_len)) {
		fputs("send_packet() error actual", stdout);
		return 0;
	}

	return 1;
}

int Client::receive_packet(void* buffer, uint16_t packet_size) {
	uint16_t net_recv;

	if (!recv_all(&net_recv, sizeof(net_recv))) {
		fputs("recv_all() error size", stdout);
		return 0;
	}

	uint16_t net_len = ntohs(net_recv);
	if (net_len > PACKET_MAX) { // was 256, since PACKET_MAX was defined as 256 but updaed to 64 after added encryption features
		fputs("packet size limit exceed", stdout);
		return 0;
	}

	if (!recv_all(buffer, net_len)) {
		fputs("recv_all() error actual", stdout);
		return 0;
	}

	return net_len;
}

int Client::Pack(mv* playerMovement, void* buffer, size_t bufferSize) {
	if (bufferSize < sizeof(mv)) {
		fputs("packet size limit exceeded", stdout);
		return 0;
	}
	memcpy(buffer, playerMovement, sizeof(mv));
	return 1;

}
