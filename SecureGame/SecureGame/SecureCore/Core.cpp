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

namespace Data {
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
}


void Run(EnclaveInput* input)
{   
    // will decrypt first later
    // moving data to namespace
    size_t offset = 0;
    Data::left_score = *(uint16_t*)(input->buffer + offset);
    offset += sizeof(uint16_t);
    Data::right_score = *(uint16_t*)(input->buffer + offset);
    offset += sizeof(uint16_t);

    Data::ball_pos_x = *(float*)(input->buffer + offset);
    offset += sizeof(float);
    Data::ball_pos_y = *(float*)(input->buffer + offset);
    offset += sizeof(float);
    Data::ball_vel_x = *(float*)(input->buffer + offset);
    offset += sizeof(float);
    Data::ball_vel_y = *(float*)(input->buffer + offset);
    offset += sizeof(float);

    Data::left_paddle_y = *(float*)(input->buffer + offset);
    offset += sizeof(float);
    Data::left_paddle_x = *(float*)(input->buffer + offset);
    offset += sizeof(float);

    Data::right_paddle_y = *(float*)(input->buffer + offset);
    offset += sizeof(float);
    Data::right_paddle_x = *(float*)(input->buffer + offset);
    offset += sizeof(float);
    
    
    /*
    const float deltaTime = currentTick->DeltaTime;

    if (currentTick->KeyW && Data::LeftPaddleY > 0)
        Data::LeftPaddleY -= PADDLE_SPEED * deltaTime;
    if (currentTick->KeyS && Data::LeftPaddleY < WINDOW_HEIGHT - PADDLE_HEIGHT)
        Data::LeftPaddleY += PADDLE_SPEED * deltaTime;
    if (currentTick->KeyUp && Data::RightPaddleY > 0)
        Data::RightPaddleY -= PADDLE_SPEED * deltaTime;
    if (currentTick->KeyDown && Data::RightPaddleY < WINDOW_HEIGHT - PADDLE_HEIGHT)
        Data::RightPaddleY += PADDLE_SPEED * deltaTime;

    Data::BallPositionX += Data::BallVelocityX * deltaTime;
    Data::BallPositionY += Data::BallVelocityY * deltaTime;

    if (Data::BallPositionY <= 0 || Data::BallPositionY + BALL_SIZE >= WINDOW_HEIGHT)
        Data::BallVelocityY = -Data::BallVelocityY;

    const bool ballInLeftPaddleYRange = Data::BallPositionY + BALL_SIZE >= Data::LeftPaddleY &&
        Data::BallPositionY <= Data::LeftPaddleY + PADDLE_HEIGHT;
    const bool ballInRightPaddleYRange = Data::BallPositionY + BALL_SIZE >= Data::RightPaddleY &&
        Data::BallPositionY <= Data::RightPaddleY + PADDLE_HEIGHT;

    if (Data::BallPositionX <= 20 + PADDLE_WIDTH &&
        Data::BallPositionX >= 20 &&
        ballInLeftPaddleYRange)
    {
        Data::BallPositionX = 20 + PADDLE_WIDTH;
        Data::BallVelocityX = -Data::BallVelocityX;
    }

    if (Data::BallPositionX + BALL_SIZE >= WINDOW_WIDTH - 20 - PADDLE_WIDTH &&
        Data::BallPositionX <= WINDOW_WIDTH - 20 &&
        ballInRightPaddleYRange)
    {
        Data::BallPositionX = WINDOW_WIDTH - 20 - PADDLE_WIDTH - BALL_SIZE;
        Data::BallVelocityX = -Data::BallVelocityX;
    }

    if (Data::BallPositionX <= 0)
    {
        Data::RightScore++;
        Data::State = Data::StateId::Reset;
    }
    if (Data::BallPositionX + BALL_SIZE >= WINDOW_WIDTH)
    {
        Data::LeftScore++;
        Data::State = Data::StateId::Reset;
    }

    */

}

extern "C" __declspec(dllexport) void* CALLBACK GameTick(PVOID context)
{

    EnclaveInput*input  = static_cast<EnclaveInput*>(context);


    /*
    switch (Data::State)
    {
    case Data::StateId::Reset:
        Reset();
        break;
    case Data::StateId::Running:
        Run(currentTick);
        break;
    }
    */

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
    currentTick->LeftPaddle.X = 20;
    currentTick->LeftPaddle.Y = Data::LeftPaddleY;
    currentTick->LeftPaddle.Width = PADDLE_WIDTH;
    currentTick->LeftPaddle.Height = PADDLE_HEIGHT;

    currentTick->RightPaddle.X = WINDOW_WIDTH - 20 - PADDLE_WIDTH;
    currentTick->RightPaddle.Y = Data::RightPaddleY;
    currentTick->RightPaddle.Width = PADDLE_WIDTH;
    currentTick->RightPaddle.Height = PADDLE_HEIGHT;

    currentTick->Ball.X = Data::BallPositionX;
    currentTick->Ball.Y = Data::BallPositionY;
    currentTick->Ball.Width = BALL_SIZE;
    currentTick->Ball.Height = BALL_SIZE;

    currentTick->LeftScore = Data::LeftScore;
    currentTick->RightScore = Data::RightScore;

    */

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

    return nullptr;
}