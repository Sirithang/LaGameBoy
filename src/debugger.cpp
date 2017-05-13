#include "debugger.h"
#include "graphic.h"
#include "gameboy.h"

using namespace debugger;


void debugger::createDebugDisplay(DebugDisplay* display, const char* title, int w, int h, int zoom)
{
	display->window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w * zoom, h * zoom, SDL_WINDOW_SHOWN);
	display->renderer = SDL_CreateRenderer(display->window, -1, SDL_RENDERER_ACCELERATED);
	display->texture = SDL_CreateTexture(display->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);
	display->pixels = new Uint32[w*h];

	display->w = w;
	display->h = h;
	display->zoom = zoom;
}

void debugger::tileDataDebug(DebugDisplay* display, Motherboard* mb)
{
	SDL_RenderClear(display->renderer);

	//tile debug
	for (int y = 0; y < 16; ++y)
	{
		for (int x = 0; x < 16; ++x)
		{
			graphic::drawTile(mb, x * 8, y * 8, (y * 16) + x, display->pixels, display->w);
		}
	}

	SDL_UpdateTexture(display->texture, NULL, &display->pixels[0], display->w * 4);
	SDL_RenderCopy(display->renderer, display->texture, NULL, NULL);
	SDL_RenderPresent(display->renderer);
}

void debugger::bgmapDebug(DebugDisplay* display, Motherboard* mb, u8 bgNum)
{
	SDL_RenderClear(display->renderer);

	u8 scrollY = motherboard::fetchu8(mb, 0xFF42);
	u8 scrollX = motherboard::fetchu8(mb, 0xFF43);

	for (int y = 0; y < 32; ++y)
	{
		for (int x = 0; x < 32; ++x)
		{
			u16 tileIdx = y * 32 + x;
			u8 tileNum = motherboard::fetchu8(mb, (bgNum == 0 ? 0x9800 : 0x9C00) + tileIdx);

			graphic::drawTile(mb, x * 8, y * 8, tileNum, display->pixels, display->w);
		}
	}

	SDL_UpdateTexture(display->texture, NULL, &display->pixels[0], display->w * 4);
	SDL_RenderCopy(display->renderer, display->texture, NULL, NULL);

	SDL_SetRenderDrawColor(display->renderer, 255, 0, 0, 255);
	SDL_Rect rect = { scrollX * display->zoom, scrollY * display->zoom, SCREEN_WIDTH * display->zoom, SCREEN_HEIGHT * display->zoom };
	SDL_RenderDrawRect(display->renderer, &rect);

	SDL_RenderPresent(display->renderer);
}