#pragma once

//#include "Client.h"
#include "Global.h"
#include "Shared.h"

#pragma warning(disable:4996);

class Client;

class Game
{
private:
    SDL_Window* m_Window = nullptr;
    SDL_Renderer* m_Renderer = nullptr;
    TTF_Font* m_Font = nullptr;
    mv* playerMove;
    gData* gameData;
    int side;
    int game_Status;
    unsigned char *send_buffer;
    unsigned char *recv_buffer;
    bool running;

    void RenderText(const char* text, int x, int y);
    void Tick(Client*);

public:
    Game();
    bool Init(Client*);
    void Loop(Client*);
    ~Game();
};