#ifndef __CLIENT_H__
#define __CLIENT_H__


#include <winsock.h>
#include <Windows.h>
#include <bcrypt.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "Shared.h"
//#include "Game.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Bcrypt.lib")

const size_t PACKET_MAX = 64;


class Client {
private:
    SOCKET sock;
    struct sockaddr_in clntAddr;
    bool connected;
    WSADATA wsaData;
    int clnt_number;
    int status;

    int send_all(const void*, size_t);
    int recv_all(void*, size_t);


public:
    Client(const char* ip, unsigned short port);
    ~Client();

    int send_packet(const void*, uint16_t);
    int receive_packet(void*, uint16_t);
    int connectToServer();
    int initial_handshake(mv*, int*, int*);
    int Pack(mv*, void*, size_t);
};

#endif

