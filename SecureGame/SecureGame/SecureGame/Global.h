#pragma once

#define ENABLE_CONSOLE true

#include <Windows.h>
#include <iostream>
#include <SDL.h>
#include <SDL_ttf.h>

#include "Game.h"
#include "Shared.h"
#include "Client.h"

#pragma comment(lib, "onecore.lib")

namespace Global
{
    inline PVOID Enclave = nullptr;
    inline PENCLAVE_ROUTINE TickRoutine = nullptr;
}