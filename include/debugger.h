#pragma once

#include "SDL.h"

struct Motherboard;

struct DebugDisplay
{
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;

	int w, h, zoom;

	Uint32* pixels;
};

namespace debugger
{
	void createDebugDisplay(DebugDisplay* display, const char* title, int w, int h, int zoom);

	void tileDataDebug(DebugDisplay* display, Motherboard* mb);
	void bgmapDebug(DebugDisplay* display, Motherboard* mb);
}