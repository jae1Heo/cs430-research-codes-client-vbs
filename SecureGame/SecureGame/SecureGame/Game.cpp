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

    EnclaveInput InputData;
    SecureZeroMemory(&InputData, sizeof(EnclaveInput));

	while(this->game_Status != 2) {
		if(this->game_Status == 0) {
			mv join = {0};
			join.player_status = 'j';

			memcpy(InputData.buffer, &join, sizeof(mv));
			SecureZeroMemory(&join, sizeof(mv));

			InputData.isEncrypt = true;

			PVOID returnValue = nullptr;
			if(!CallEnclave(Global::TickRoutine, &InputData, true, &returnValue)) {
				char buffer[256];
                sprintf_s(buffer, "Failed to call enclave routine: %d", GetLastError());
                MessageBoxA(nullptr, buffer, "Error", MB_OK | MB_ICONERROR);
                this->running = false;
				return false;
			}

         
            unsigned char buffer[sizeof(envelope)] = { 0 };
            memcpy(buffer, InputData.buffer, sizeof(envelope));

			if(!client->send_packet((unsigned char*)InputData.buffer, sizeof(envelope))) {
				this->running = false;
				return false;
			}
			SecureZeroMemory(&InputData, sizeof(EnclaveInput));
			this->game_Status = 1;
		}
		else if(this->game_Status == 1) {
			unsigned char recv_buffer[sizeof(envelope)] = {0};
			int recv_len = client->receive_packet(recv_buffer, sizeof(envelope));
			if(recv_len != sizeof(envelope)) {
				return false;
			}

			memcpy(InputData.buffer, recv_buffer, sizeof(envelope));
			SecureZeroMemory(recv_buffer, sizeof(envelope));
			InputData.isEncrypt = false;

			PVOID returnValue = nullptr;
			if(!CallEnclave(Global::TickRoutine, &InputData, true, &returnValue)) {
				char buffer[256];
                sprintf_s(buffer, "Failed to call enclave routine: %d", GetLastError());
                MessageBoxA(nullptr, buffer, "Error", MB_OK | MB_ICONERROR);
                this->running = false;
				return false;
			}

			if(InputData.buffer[0] == 's') {
				this->side = (int)InputData.buffer[1];

				mv ack = {0};
				ack.player_status = 'a';

				SecureZeroMemory(&InputData, sizeof(EnclaveInput));
				memcpy(InputData.buffer, &ack, sizeof(mv));
				InputData.isEncrypt = true;

				if(!CallEnclave(Global::TickRoutine, &InputData, true, &returnValue)) {
					char buffer[256];
	                sprintf_s(buffer, "Failed to call enclave routine: %d", GetLastError());
	                MessageBoxA(nullptr, buffer, "Error", MB_OK | MB_ICONERROR);
	                this->running = false;
					return false;
				}

				if(!client->send_packet((unsigned char*)InputData.buffer, sizeof(envelope))) {
					this->running = false;
					return false;
				}

				SecureZeroMemory(&InputData, sizeof(EnclaveInput));
				this->game_Status = 2;
			}
			else {
				MessageBoxA(nullptr, "Handshake failed", "Error", MB_OK | MB_ICONERROR);
		        fputs("handshake failed\n", stderr);
		        return false;
			}
		}
		else {
			MessageBoxA(nullptr, "Handshake failed", "Error", MB_OK | MB_ICONERROR);
        	fputs("handshake failed\n", stderr);
        	return false;
		}
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

    if (this->game_Status != 2) {
        MessageBoxA(nullptr, "Invalid game option", "Error", MB_OK | MB_ICONERROR);
        this->running = false;
    }
    const Uint8* keystates = SDL_GetKeyboardState(nullptr);
	mv playerMove = {0};
    playerMove.player_status = 'p';
    if (keystates[SDL_SCANCODE_W]) {
        playerMove.player_w = 1;
        playerMove.player_s = 0;
    }
    else if (keystates[SDL_SCANCODE_S]) {
        playerMove.player_w = 0;
        playerMove.player_s = 1;
    }
    else {
        playerMove.player_w = 0;
        playerMove.player_s = 0;
    }

    EnclaveInput InputData = {0};
	memcpy(InputData.buffer, &playerMove, sizeof(mv));
	//SecureZeroMemory(&InputData, sizeof(EnclaveInput));
	
	InputData.isEncrypt = true;

	PVOID returnValue = nullptr;
	if (!CallEnclave(Global::TickRoutine, &InputData, true, &returnValue)) {
		char buffer[256];
		sprintf_s(buffer, "Failed to call enclave routine: %d", GetLastError());
		MessageBoxA(nullptr, buffer, "Error", MB_OK | MB_ICONERROR);
		this->running = false;
        return;
	}

	if (!client->send_packet(InputData.buffer, sizeof(envelope))) {
		MessageBoxA(nullptr, "Failed to send the packet", "Error", MB_OK | MB_ICONERROR);
		this->running = false;
		return;
	}
    

	unsigned char recv_buffer[sizeof(envelope)];

    int recv_len = client->receive_packet(recv_buffer, sizeof(envelope));
	
    if (recv_len != sizeof(envelope)) {
        MessageBoxA(nullptr, "Failed to receive data from the server", "Error", MB_OK | MB_ICONERROR);
        this->running = false;
        return;
    }

    SecureZeroMemory(&InputData, sizeof(EnclaveInput));
    memcpy(InputData.buffer, recv_buffer, sizeof(envelope));
	SecureZeroMemory(recv_buffer, sizeof(envelope));
    InputData.isEncrypt = false;

    returnValue = nullptr;
    if (!CallEnclave(Global::TickRoutine, &InputData, true, &returnValue)) {
        char buffer[256];
        sprintf_s(buffer, "Failed to call enclave routine: %d", GetLastError());
        MessageBoxA(nullptr, buffer, "Error", MB_OK | MB_ICONERROR);
        this->running = false;
        return;
    }
	EnclaveOutput renderData;
	memcpy(&renderData, InputData.buffer, sizeof(EnclaveOutput));
	SecureZeroMemory(InputData.buffer, sizeof(envelope));

	if(!renderData.valid) {
		this->running = false;
		return;
	}

    SDL_SetRenderDrawColor(m_Renderer, 255, 255, 255, 255);
	
    SDL_RenderFillRect(m_Renderer, (const SDL_Rect*)&renderData.rects[0]);

    SDL_RenderFillRect(m_Renderer, (const SDL_Rect*)&renderData.rects[1]);

    SDL_RenderFillRect(m_Renderer, (const SDL_Rect*)&renderData.rects[2]);

    char scoreText[32];
    sprintf_s(scoreText, "%d - %d", renderData.left_score, renderData.right_score);
    RenderText(scoreText, WINDOW_WIDTH / 2 - 40, 20);
	
	SDL_RenderPresent(m_Renderer);

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
