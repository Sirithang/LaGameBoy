#pragma once

#include "SDL_stdinc.h"
#include "utils.h"

struct Motherboard;

struct GPU
{
	int currentTick;
	//0:HBLANK, 1:VBLANK, 2:OAM ACCESS, 3:VRAM access 
	u8 currentState;
	u8 currentLine;

	//use uint32 as will be fed diretly to the texture, would prefer to work with u8 :/
	Uint32 buffer[SCREEN_WIDTH*SCREEN_HEIGHT];

	Motherboard* mb;
};


extern Uint32 palette[4];

namespace gpu
{
	void init(GPU* gpu);

	//return true if the display buffer must be displayed on screen (VBLANK triggered)
	bool tick(GPU* gpu, int tickCount);

	void renderLine(GPU* gpu, u8 line);
}

namespace graphic
{
	u8 fetchTilePixelPaletteIdx(Motherboard* motherboard, u8 tileNum, u8 pixelY, u8 pixelX, u8 type);
	void drawTile(Motherboard* motherboard, u16 x, u16 y, u8 tilenum, u8 type, Uint32* pixels, u16 width);
}