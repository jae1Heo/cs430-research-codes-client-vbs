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
	const unsigned char* p = (const unsigned char*)data;

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
	const unsigned char* p = (const unsigned char*)data;

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
	if (net_len == 0 || net_len > packet_size) {
		fputs("packet size limit exceed", stdout);
		return 0;
	}

	if (!recv_all(buffer, net_len)) {
		fputs("recv_all() error actual", stdout);
		return 0;
	}
}

int Client::Pack(mv* playerMovement, void* buffer, size_t bufferSize) {
	if (bufferSize < sizeof(mv)) {
		fputs("packet size limit exceeded", stdout);
		return 0;
	}
	memcpy(buffer, playerMovement, sizeof(mv));
	return 1;

}

int Client::initial_handshake(mv* playerMovement, int* game_status, int* side) {
	unsigned char* buffer = (unsigned char*)malloc(PACKET_MAX);
	memset((void*)buffer, 0, PACKET_MAX);
	
	if (*game_status > 0) {
		fputs("handshake failed\n", stderr);
		free(buffer);
		return 0;
	}

	while (*game_status != 2) {
		if (*game_status == 0) {
			playerMovement->player_status = 'j';
			playerMovement->player_w = 0;
			playerMovement->player_s = 0;

			if (!Pack(playerMovement, (void*)buffer, sizeof(mv))) {
				fputs("error packing data\n", stderr);
				free(buffer);
				return 0;
			}

			if (!send_packet(buffer, sizeof(mv))) {
				fputs("error sending packet\n", stderr);
				free(buffer);
				return 0;
			}

			memset((void*)buffer, 0, PACKET_MAX);
			*game_status = 1;
		}
		else if (*game_status == 1) {
			if (!receive_packet(buffer, sizeof(mv))) {
				fputs("error receiving packet\n", stderr);
				free(buffer);
				return 0;
			}

			fputs("successfully joined the game\n", stdout);
			if (buffer[0] == 's') {
				*side = (int)buffer[1];
				playerMovement->player_status = 'a';
				playerMovement->player_w = 0;
				playerMovement->player_s = 0;

				if(!Pack(playerMovement, buffer, sizeof(mv))) {
					fputs("error packing data\n", stderr);
					free(buffer);
					return 0;
				}

				if (!send_packet(buffer, sizeof(mv))) {
					fputs("error sending packet\n", stderr);
					free(buffer);
					return 0;
				}

				*game_status = 2;
			}
			else {
				fputs("invalid packet\n", stderr);
				free(buffer);
				return 0;
			}
		}
	}

	return 1;
}