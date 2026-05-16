#include "Game.h"

Game::Game() {
    this->playerMove = (mv*)malloc(sizeof(mv));
    this->gameData = (gData*)malloc(sizeof(gData));
    this->send_buffer = (unsigned char*)malloc(PACKET_MAX);
    this->recv_buffer = (unsigned char*)malloc(PACKET_MAX);
    this->game_Status = 0;
    this->side = 0;
    this->running = true;
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

    //int hs = client->initial_handshake(this->playerMove, &this->game_Status, &this->side);

    /*
    char msg[128];
    memset(msg, 0, 128);
    if (hs) {
        sprintf(msg, "Failed to complete handshake : %d", hs);
        MessageBoxA(nullptr, msg, "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    */

    //unsigned char* buffer = (unsigned char*)malloc(PACKET_MAX);
    //memset((void*)buffer, 0, PACKET_MAX);
    EnclaveInput InputData;
    memset(&InputData, 0, sizeof(EnclaveInput));

    if (this->game_Status > 0) {
		MessageBoxA(nullptr, "Handshake failed", "Error", MB_OK | MB_ICONERROR);
        fputs("handshake failed\n", stderr);
        //free(buffer);
        return false;
    }
    while (this->game_Status != 2) {
        if (this->game_Status == 0) {
            playerMove->player_status = 'j';
            playerMove->player_w = 0;
            playerMove->player_s = 0;

            if (!client->Pack(playerMove, (void*)InputData.buffer, sizeof(mv))) {
				MessageBoxA(nullptr, "Error packing data", "Error", MB_OK | MB_ICONERROR);
                //free(buffer);
                return false;
            }

            InputData.isEncrypt = true;
            InputData.cipherLen = 0;
            PVOID returnValue = nullptr;
            if (!CallEnclave(Global::TickRoutine, &InputData, true, &returnValue)) {
                char buffer[256];
                sprintf_s(buffer, "Failed to call enclave routine: %d", GetLastError());
                MessageBoxA(nullptr, buffer, "Error", MB_OK | MB_ICONERROR);
                this->running = false;
            }

            if (!client->send_packet(InputData.buffer, PACKET_MAX)) {
                fputs("error sending packet\n", stderr);
                //free(buffer);
                return false;
            }
            //memset((void*)buffer, 0, PACKET_MAX);
            memset(InputData.buffer, 0, PACKET_MAX);
            this->game_Status = 1;
        }
        else if (this->game_Status == 1) {
			int recv_len = client->receive_packet(InputData.buffer, PACKET_MAX);
            if (!recv_len) {
                fputs("error receiving packet\n", stderr);
                //free(buffer);
				return false;
            }
            InputData.isEncrypt = false;
			InputData.cipherLen = recv_len;
            //memcpy(InputData.buffer, buffer, sizeof(InputData.buffer));
            // sending packet so encryption required
            PVOID returnValue = nullptr;
            if (!CallEnclave(Global::TickRoutine, &InputData, true, &returnValue)) {
                char buffer[256];
                sprintf_s(buffer, "Failed to call enclave routine: %d", GetLastError());
                MessageBoxA(nullptr, buffer, "Error", MB_OK | MB_ICONERROR);
                this->running = false;
            }


            if (InputData.buffer[0] == 's') {
                this->side = (int)InputData.buffer[1];
                playerMove->player_status = 'a';
                playerMove->player_w = 0;
                playerMove->player_s = 0;
				memset(InputData.buffer, 0, PACKET_MAX);
                if (!client->Pack(playerMove, InputData.buffer, sizeof(mv))) {
                    fputs("error packing data\n", stderr);
                    //free(buffer);
                    return false;
                }

				InputData.isEncrypt = true;
                InputData.cipherLen = 0;
                PVOID returnValue = nullptr;
                if (!CallEnclave(Global::TickRoutine, &InputData, true, &returnValue)) {
                    char buffer[256];
                    sprintf_s(buffer, "Failed to call enclave routine: %d", GetLastError());
                    MessageBoxA(nullptr, buffer, "Error", MB_OK | MB_ICONERROR);
                    this->running = false;
                }


                if (!client->send_packet(InputData.buffer, PACKET_MAX)) {
                    fputs("error sending packet\n", stderr);
                    //free(buffer);
                    return false;
                }
                
				memset(InputData.buffer, 0, PACKET_MAX);
                this->game_Status = 2;
            }
            else {
                fputs("invalid packet\n", stderr);
                //free(buffer);
                return false;
            }
		}
    }

    /*
    memset(InputData.buffer, 0, PACKET_MAX);

    this->playerMove->player_status = 'p';
    this->playerMove->player_w = 0;
    this->playerMove->player_s = 0;

    if (!client->Pack(this->playerMove, InputData.buffer, sizeof(mv))) {
        MessageBoxA(nullptr, "Failed to pack the data", "Error", MB_OK | MB_ICONERROR);
        this->running = false;
    }

    InputData.isEncrypt = true;
    PVOID returnValue = nullptr;
    if (!CallEnclave(Global::TickRoutine, &InputData, true, &returnValue)) {
        char buffer[256];
        sprintf_s(buffer, "Failed to call enclave routine: %d", GetLastError());
        MessageBoxA(nullptr, buffer, "Error", MB_OK | MB_ICONERROR);
        this->running = false;
    }

    if (!client->send_packet(InputData.buffer, PACKET_MAX)) {
        MessageBoxA(nullptr, "Failed to send the packet", "Error", MB_OK | MB_ICONERROR);
        this->running = false;
    }

    memset(this->send_buffer, 0, PACKET_MAX);
    */
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

    if (this->game_Status != 2) {
        MessageBoxA(nullptr, "Invalid game option", "Error", MB_OK | MB_ICONERROR);
        this->running = false;
    }
    const Uint8* keystates = SDL_GetKeyboardState(nullptr);

    playerMove->player_status = 'p';
    if (keystates[SDL_SCANCODE_W]) {
        playerMove->player_w = 1;
        playerMove->player_s = 0;
    }
    else if (keystates[SDL_SCANCODE_S]) {
        playerMove->player_w = 1;
        playerMove->player_s = 0;
    }
    else {
        playerMove->player_w = 0;
        playerMove->player_s = 0;
    }

    EnclaveInput InputData;
    memset(&InputData, 0, sizeof(EnclaveInput));

    if (!client->Pack(this->playerMove, this->send_buffer, sizeof(mv))) {
		MessageBoxA(nullptr, "Failed to pack the data", "Error", MB_OK | MB_ICONERROR);
        this->running = false;
        return;
    }

	memcpy(InputData.buffer, this->send_buffer, PACKET_MAX);
	InputData.isEncrypt = true;
	InputData.cipherLen = 0;

	PVOID returnValue = nullptr;
	if (!CallEnclave(Global::TickRoutine, &InputData, true, &returnValue)) {
		char buffer[256];
		sprintf_s(buffer, "Failed to call enclave routine: %d", GetLastError());
		MessageBoxA(nullptr, buffer, "Error", MB_OK | MB_ICONERROR);
		this->running = false;
        return;
	}

	if (!client->send_packet(InputData.buffer, PACKET_MAX)) {
		MessageBoxA(nullptr, "Failed to send the packet", "Error", MB_OK | MB_ICONERROR);
		this->running = false;
		return;
	}
    

	memset(recv_buffer, 0, PACKET_MAX);

    int cipher_len = client->receive_packet(recv_buffer, PACKET_MAX);
    if (!cipher_len) {
        MessageBoxA(nullptr, "Failed to receive data from the server", "Error", MB_OK | MB_ICONERROR);
        this->running = false;
        return;
    }

    memset(&InputData, 0, sizeof(EnclaveInput));
    memcpy(InputData.buffer, recv_buffer, PACKET_MAX);
    InputData.isEncrypt = false;

    returnValue = nullptr;
    InputData.cipherLen = cipher_len;
    if (!CallEnclave(Global::TickRoutine, &InputData, true, &returnValue)) {
        char buffer[256];
        sprintf_s(buffer, "Failed to call enclave routine: %d", GetLastError());
        MessageBoxA(nullptr, buffer, "Error", MB_OK | MB_ICONERROR);
        this->running = false;
        return;
    }
	memcpy(this->gameData, InputData.buffer, sizeof(gData));

    SDL_SetRenderDrawColor(m_Renderer, 255, 255, 255, 255);
    const SDL_Rect leftPaddle =
    {
        static_cast<int>(gameData->left_paddle_x),
        static_cast<int>(gameData->left_paddle_y),
        static_cast<int>(PADDLE_WIDTH),
        static_cast<int>(PADDLE_HEIGHT)
    };
    SDL_RenderFillRect(m_Renderer, &leftPaddle);

    const SDL_Rect rightPaddle =
    {
        static_cast<int>(gameData->right_paddle_x),
        static_cast<int>(gameData->right_paddle_y),
        static_cast<int>(PADDLE_WIDTH),
        static_cast<int>(PADDLE_HEIGHT)
    };
    SDL_RenderFillRect(m_Renderer, &rightPaddle);

    const SDL_Rect ball =
    {
        static_cast<int>(gameData->ball_pos_x),
        static_cast<int>(gameData->ball_pos_y),
        static_cast<int>(BALL_SIZE),
        static_cast<int>(BALL_SIZE)
    };
    SDL_RenderFillRect(m_Renderer, &ball);

    char scoreText[32];
    //sprintf_s(scoreText, "%d - %d", InputData.state.left_score, InputData.state.right_score);
    //RenderText(scoreText, WINDOW_WIDTH / 2 - 40, 20);

   

    memset(recv_buffer, 0, PACKET_MAX);
    memset(send_buffer, 0, PACKET_MAX);
    memset(this->playerMove, 0, sizeof(mv));
    memset(this->gameData, 0, sizeof(gData));

}

void Game::Loop(Client* client)
{
    constexpr int FPS = 120;
    constexpr int frameDelay = 1000 / FPS;


    while (this->running)
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