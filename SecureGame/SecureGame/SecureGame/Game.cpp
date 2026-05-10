#include "Game.h"

Game::Game() {
    this->playerMove = (mv*)malloc(sizeof(mv));
    this->gameData = (gData*)malloc(sizeof(gData));
    this->send_buffer = (unsigned char*)malloc(PACKET_MAX);
    this->recv_buffer = (unsigned char*)malloc(PACKET_MAX);
    this->game_Status = 0;
    this->side = 0;
}

Game::~Game() {
    free(this->playerMove);
    free(this->gameData);
}

bool Game::Init(Client* client)
{
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        MessageBoxA(nullptr, "Failed to initialize SDL", "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    if (TTF_Init())
    {
        MessageBoxA(nullptr, "Failed to initialize SDL_ttf", "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    m_Font = TTF_OpenFont(R"(C:\Windows\Fonts\Arial.ttf)", 24);
    if (!m_Font)
    {
        MessageBoxA(nullptr, "Failed to load font", "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    m_Window = SDL_CreateWindow("SecureGame", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!m_Window)
    {
        MessageBoxA(nullptr, "Failed to create window", "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    m_Renderer = SDL_CreateRenderer(m_Window, -1, SDL_RENDERER_ACCELERATED);
    if (!m_Renderer)
    {
        MessageBoxA(nullptr, "Failed to create renderer", "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    if (!client->connectToServer()) {
        MessageBoxA(nullptr, "Failed to connect to server", "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    if (!client->initial_handshake(this->playerMove, &this->game_Status, &this->side)) {
        MessageBoxA(nullptr, "Failed to complete handshake", "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    return true;
}

void Game::RenderText(const char* text, int x, int y)
{
    constexpr SDL_Color textColor = { 255, 255, 255, 255 };
    if (SDL_Surface* surface = TTF_RenderText_Solid(this->m_Font, text, textColor))
    {
        if (SDL_Texture* texture = SDL_CreateTextureFromSurface(this->m_Renderer, surface))
        {
            const SDL_Rect destRect = { x, y, surface->w, surface->h };
            SDL_RenderCopy(this->m_Renderer, texture, nullptr, &destRect);
            SDL_DestroyTexture(texture);
        }

        SDL_FreeSurface(surface);
    }
}

void Game::Tick(Client* client)
{
    const Uint8* keystates = SDL_GetKeyboardState(nullptr);

    if (this->game_Status != 2) {
        MessageBoxA(nullptr, "Invalid game option", "Error", MB_OK | MB_ICONERROR);
        return;
    }
    else {
        if (!client->receive_packet(recv_buffer, sizeof(gData))) {
            MessageBoxA(nullptr, "Failed to receive data from the server", "Error", MB_OK | MB_ICONERROR);
            return;
        }
    }

    EnclaveInput InputData;
    memcpy(InputData.buffer, recv_buffer, PACKET_MAX);

    PVOID returnValue = nullptr;
    if (!CallEnclave(Global::TickRoutine, &InputData, true, &returnValue)) {
        char buffer[256];
        sprintf_s(buffer, "Failed to call enclave routine: %d", GetLastError());
        MessageBoxA(nullptr, buffer, "Error", MB_OK | MB_ICONERROR);
        return;
    }

    SDL_SetRenderDrawColor(m_Renderer, 255, 255, 255, 255);
    const SDL_Rect leftPaddle =
    {
        static_cast<int>(InputData.state.left_paddle_x),
        static_cast<int>(InputData.state.left_paddle_y),
        static_cast<int>(PADDLE_WIDTH),
        static_cast<int>(PADDLE_HEIGHT)
    };
    SDL_RenderFillRect(m_Renderer, &leftPaddle);

    const SDL_Rect rightPaddle =
    {
        static_cast<int>(InputData.state.right_paddle_x),
        static_cast<int>(InputData.state.right_paddle_y),
        static_cast<int>(PADDLE_WIDTH),
        static_cast<int>(PADDLE_HEIGHT)
    };
    SDL_RenderFillRect(m_Renderer, &rightPaddle);

    const SDL_Rect ball =
    {
        static_cast<int>(InputData.state.ball_pos_x),
        static_cast<int>(InputData.state.ball_pos_y),
        static_cast<int>(BALL_SIZE),
        static_cast<int>(BALL_SIZE)
    };
    SDL_RenderFillRect(m_Renderer, &ball);

    char scoreText[32];
    sprintf_s(scoreText, "%d - %d", InputData.state.left_score, InputData.state.right_score);
    RenderText(scoreText, WINDOW_WIDTH / 2 - 40, 20);

    if (keystates[SDL_SCANCODE_W]) {
        playerMove->player_w = 1;
        playerMove->player_s = 0;
    }
    else if (keystates[SDL_SCANCODE_S]) {
        playerMove->player_s = 1;
        playerMove->player_w = 0;
    }

    this->playerMove->player_status = 'p';

    if (!client->Pack(this->playerMove, send_buffer, sizeof(mv))) {
        MessageBoxA(nullptr, "Failed to pack the data", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    if (!client->send_packet((void*)send_buffer, sizeof(mv))) {
        MessageBoxA(nullptr, "Failed to send the packet", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    memset(recv_buffer, 0, PACKET_MAX);
    memset(send_buffer, 0, PACKET_MAX);

    //data.KeyW = keystates[SDL_SCANCODE_W];
    //data.KeyS = keystates[SDL_SCANCODE_S];

    // not in use anymore
    //data.KeyUp = keystates[SDL_SCANCODE_UP];
    //data.KeyDown = keystates[SDL_SCANCODE_DOWN];

    /*
    PVOID returnValue = nullptr;
    if (!CallEnclave(Global::TickRoutine, &data, true, &returnValue))
    {
        char buffer[256];
        sprintf_s(buffer, "Failed to call enclave routine: %d", GetLastError());
        MessageBoxA(nullptr, buffer, "Error", MB_OK | MB_ICONERROR);
        return;
    }

    SDL_SetRenderDrawColor(m_Renderer, 255, 255, 255, 255);

    const SDL_Rect leftPaddle =
    {
        static_cast<int>(data.LeftPaddle.X),
        static_cast<int>(data.LeftPaddle.Y),
        static_cast<int>(data.LeftPaddle.Width),
        static_cast<int>(data.LeftPaddle.Height)
    };
    SDL_RenderFillRect(m_Renderer, &leftPaddle);

    const SDL_Rect rightPaddle =
    {
        static_cast<int>(data.RightPaddle.X),
        static_cast<int>(data.RightPaddle.Y),
        static_cast<int>(data.RightPaddle.Width),
        static_cast<int>(data.RightPaddle.Height)
    };
    SDL_RenderFillRect(m_Renderer, &rightPaddle);

    const SDL_Rect ball =
    {
        static_cast<int>(data.Ball.X),
        static_cast<int>(data.Ball.Y),
        static_cast<int>(data.Ball.Width),
        static_cast<int>(data.Ball.Height)
    };
    SDL_RenderFillRect(m_Renderer, &ball);

    char scoreText[32];
    sprintf_s(scoreText, "%d - %d", data.LeftScore, data.RightScore);
    RenderText(scoreText, WINDOW_WIDTH / 2 - 40, 20);

    */



}

void Game::Loop(Client* client)
{
    constexpr int FPS = 240;
    constexpr int frameDelay = 1000 / FPS;

    bool running = true;

    while (running)
    {
        const Uint32 frameStart = SDL_GetTicks();

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = false;
        }

        SDL_SetRenderDrawColor(m_Renderer, 0, 0, 0, 255);
        SDL_RenderClear(m_Renderer);

        Tick(client);

        SDL_RenderPresent(m_Renderer);

        const int frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime)
            SDL_Delay(frameDelay - frameTime);
    }

    SDL_DestroyRenderer(m_Renderer);
    SDL_DestroyWindow(m_Window);
    SDL_Quit();
}