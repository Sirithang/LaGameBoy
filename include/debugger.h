#pragma once

#include "SDL.h"
#include "utils.h"

struct Motherboard;

struct DebugDisplay
{
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;

	int w, h, zoom;

	Uint32* pixels;
};

struct CallHistory
{
	u16 PCHistory[256];
	u8 currentCall;
};

namespace debugger
{
	void createDebugDisplay(DebugDisplay* display, const char* title, int w, int h, int zoom);

	void tileDataDebug(DebugDisplay* display, Motherboard* mb);
	void bgmapDebug(DebugDisplay* display, Motherboard* mb, u8 bgNum);
}