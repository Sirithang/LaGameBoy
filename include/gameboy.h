#pragma once

#include "SDL.h"

const int SCREEN_WIDTH = 160;
const int SCREEN_HEIGHT = 144;

typedef unsigned char u8;
typedef unsigned short u16;

struct VideoController
{
	u8 VRAM[0x1FFF];
	SDL_Texture* displayBuffer;
};

struct MemoryController
{
	VideoController vcontroller;
};


namespace memc
{
	u8 fetchu8(MemoryController& controller, u16 address);
	u16 fetchu16(MemoryController& controller, u16 address);

	void writeu8(MemoryController& controller, u16 address, u8 value);
	void writeu16(MemoryController& controller, u16 address, u16 value);
}

namespace videocontroller
{
	void init(VideoController& vcontroller, SDL_Renderer* renderer);
	void copyVRAMToBuffer(VideoController& vcontroller);
}