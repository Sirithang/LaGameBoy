#pragma once

#include "SDL.h"

const int SCREEN_WIDTH = 160;
const int SCREEN_HEIGHT = 144;

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed char s8;

struct InternalMemory
{ //all memeory size have +1, because the size have been computed as ending_adress - starting_adress. but ending adress is included.
	u8 VRAM[0x1FFF + 1];
	u8 OAM[0x9F + 1];
	u8 WRAM[0x1FFF + 1];
	u8 IORegister[0x7F + 1];
	u8 HRAM[0x7E + 1];
	u8 InterruptRegister;
};


struct GPU
{
	int currentTick;
	//0:HBLANK, 1:VBLANK, 2:OAM ACCESS, 3:VRAM access 
	u8 currentState;
	u8 currentLine;
};

struct Cart
{
	u8* content;
	//TODO handle MBC
};

struct Addresser
{
	InternalMemory internalMemory;
	GPU gpu;
	Cart cart;
};

namespace cart
{
	void load(Cart& cart, const char* path);
	u8* address(Cart& cart, u16 address);
}

namespace gpu
{
	void init(GPU& gpu);

	//return true if the display buffer must be displayed on screen (VBLANK triggered)
	bool tick(GPU& gpu, int tickCount);
}

namespace addresser
{
	u8 fetchu8(Addresser& addresser, u16 address);
	s8 fetchs8(Addresser& addresser, u16 address);
	u16 fetchu16(Addresser& addresser, u16 address);

	void writeu8(Addresser& addresser, u16 address, u8 value);
	void writeu16(Addresser& addresser, u16 address, u16 value);

	void updateGPURegister(Addresser& addresser);
}