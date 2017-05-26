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
	struct Call
	{
		u16 address;
		u8 opcode;
		u8 extendedOpcode;
		u8 interupted;
		u8 halted;
	};

	Call PCHistory[256];
	
	u8 currentCall;
};

namespace debugger
{
	extern CallHistory s_callHistory;

	void createDebugDisplay(DebugDisplay* display, const char* title, int w, int h, int zoom);

	void tileDataDebug(DebugDisplay* display, Motherboard* mb, u8 type);
	void bgmapDebug(DebugDisplay* display, Motherboard* mb, u8 bgNum, u8 type);

	CallHistory::Call* getCurrentCallData();
}