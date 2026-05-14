#include "Global.h"

/*
namespace Data
{
    enum class StateId
    {
        Reset,
        Running,
    };

    StateId State = StateId::Reset;

    int LeftScore = 0;
    int RightScore = 0;

    float BallPositionX = 0;
    float BallPositionY = 0;

    float BallVelocityX = 0;
    float BallVelocityY = 0;

    float LeftPaddleY = static_cast<float>(WINDOW_HEIGHT) / 2 - PADDLE_HEIGHT / 2;
    float RightPaddleY = static_cast<float>(WINDOW_HEIGHT) / 2 - PADDLE_HEIGHT / 2;
}

void Reset()
{
    Data::BallPositionX = static_cast<float>(WINDOW_WIDTH) / 2 - BALL_SIZE / 2;
    Data::BallPositionY = static_cast<float>(WINDOW_HEIGHT) / 2 - BALL_SIZE / 2;

    Data::BallVelocityX = (rand() % 2 == 0) ? BALL_SPEED : -BALL_SPEED;
    Data::BallVelocityY = (rand() % 2 == 0) ? BALL_SPEED : -BALL_SPEED;

    Data::State = Data::StateId::Running;
}
*/


extern "C" __declspec(dllexport) void* CALLBACK GameTick(PVOID context)
{

    if (!context) {
        return nullptr;
    }
    EnclaveInput*input  = static_cast<EnclaveInput*>(context);
    Encryption enc = Encryption();


    if(input->isEncrypt) {
        unsigned char ciphertext[64];
        unsigned char plaintext[64];
        int plaintext_len = sizeof(playerMV);

		memcpy(plaintext, input->buffer, plaintext_len);

		int ciphertext_len = enc.AES_encrypt(plaintext, plaintext_len, ciphertext);
        if (ciphertext_len < 0) {
            return nullptr;
        }

        memset(input->buffer, 0, 64);
		memcpy(input->buffer, ciphertext, ciphertext_len);
        input->isEncrypt = false;
	}
    else {
        unsigned char plaintext[64];
		if (input->cipherLen <= 0) {
            return nullptr;
        }
        int plaintext_len = enc.AES_decrypt((unsigned char*)input->buffer, input->cipherLen, plaintext);
        if (plaintext_len < 0) {
            return nullptr;
        }

        memset(input->buffer, 0, sizeof(64));
        memcpy(input->buffer, plaintext, plaintext_len);
    }


    /*
     * We do not read the data directly and instead save them
     * in the Data namespace so that they cannot be tampered with.
     * If we did, user could just change the values out of the enclave
     * and this would defeat the purpose of the enclave.
     *
     * Of course there are million different ways you can tamper with those
     * data, but this is just a PoC :)
     */
    
    /*
    input->state.left_score = Data::left_score;
    input->state.right_score = Data::right_score;
    input->state.ball_pos_x = Data::ball_pos_x;
    input->state.ball_pos_y = Data::ball_pos_y;
    input->state.ball_vel_x = Data::ball_vel_x;
    input->state.ball_vel_y = Data::ball_vel_y;
    input->state.left_paddle_y = Data::left_paddle_y;
    input->state.left_paddle_x = Data::left_paddle_x;
    input->state.right_paddle_y = Data::right_paddle_y;
    input->state.right_paddle_x = Data::right_paddle_x;
    */

    return nullptr;
}