#pragma once

#include <stdint.h>

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;

constexpr float BALL_SPEED = 150.0f;
constexpr float PADDLE_SPEED = 300.0f;
constexpr float PADDLE_WIDTH = 15.0f;
constexpr float PADDLE_HEIGHT = 90.0f;
constexpr float PADDLE_WIDTH_PADDING = 20.0f;
constexpr float BALL_SIZE = 15.0f;

#define RSA_SIGNED_SIZE 256
#define PACKET_MAX 64
#define SYMKEY_SIZE 32
#define IV_SIZE 16
#define HASH_SIZE_BEFORE_SIGN 32

#pragma pack(push, 1)
typedef struct playerMV {
    uint8_t player_status;
    uint8_t player_w;
    uint8_t player_s;
}mv;

typedef struct gameData {
    uint16_t left_score;
    uint16_t right_score;

    float ball_pos_x;
    float ball_pos_y;
    float ball_vel_x;
    float ball_vel_y;

    float left_paddle_y;
    float left_paddle_x;
    float right_paddle_y;
    float right_paddle_x;
}gData;

typedef struct {
    unsigned char symkey[RSA_SIGNED_SIZE];
    unsigned char iv[IV_SIZE];
    unsigned char packet[PACKET_MAX];
    uint16_t packet_len;
    unsigned char hash_signed[RSA_SIGNED_SIZE];
}envelope;

typedef struct EnclaveInput {
    char buffer[64];
    int cipherLen;
    bool isEncrypt;
}EnclaveInput;
#pragma pack(pop)

