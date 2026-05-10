#pragma once

#include <stdint.h>

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;
constexpr int PACKET_DATA = 256;

constexpr float BALL_SPEED = 150.0f;
constexpr float PADDLE_SPEED = 300.0f;
constexpr float PADDLE_WIDTH = 15.0f;
constexpr float PADDLE_HEIGHT = 90.0f;
constexpr float PADDLE_WIDTH_PADDING = 20.0f;
constexpr float BALL_SIZE = 15.0f;


/*
typedef struct _TICK_INPUT
{
    float DeltaTime;

    bool KeyW;
    bool KeyS;
    bool KeyUp;
    bool KeyDown;

    struct
    {
        float X;
        float Y;
        float Width;
        float Height;
    } LeftPaddle, RightPaddle, Ball;

    int LeftScore;
    int RightScore;
} TICK_DATA;
*/

/*
typedef struct _TICK_INPUT {
    float DeltaTime;
}TICK_DATA;
*/

#pragma pack(push, 1)
typedef struct playerMV {
    uint8_t player_status;
    uint8_t player_w;
    uint8_t player_s;
}mv;
#pragma pack(pop)

#pragma pack(push, 1)
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
#pragma pack(pop)

typedef struct EnclaveInput {
    char buffer[PACKET_DATA];
    gData state;
}EnclaveInput;