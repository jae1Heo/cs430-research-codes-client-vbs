#pragma once

//#include "Client.h"
#include "Global.h"
#include "Shared.h"

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

    void RenderText(const char* text, int x, int y);
    void Tick(Client*);

public:
    Game();
    bool Init(Client*);
    void Loop(Client*);
    ~Game();
};