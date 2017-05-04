#pragma once

#include "SDL.h"

const int SCREEN_WIDTH = 160;
const int SCREEN_HEIGHT = 144;

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed char s8;

struct VideoController
{
	u8 VRAM[0x1FFF];
	SDL_Texture* displayBuffer;
};

struct InternalMemory
{
	u8 VRAM[0x1FFF];
	u8 OAM[0x9F];
	u8 WRAM[0x1FFF];
	u8 IORegister[0x7F];
	u8 HRAM[0x7E];
	u8 InterruptRegister;
};

struct Cart
{
	u8* content;
	//TODO handle MBC
};

struct Addresser
{
	InternalMemory internalMemory;
	Cart cart;
};

namespace cart
{
	void load(Cart& cart, const char* path);
	u8* address(Cart& cart, u16 address);
}

namespace addresser
{
	u8 fetchu8(Addresser& addresser, u16 address);
	s8 fetchs8(Addresser& addresser, u16 address);
	u16 fetchu16(Addresser& addresser, u16 address);

	void writeu8(Addresser& addresser, u16 address, u8 value);
	void writeu16(Addresser& addresser, u16 address, u16 value);
}

namespace videocontroller
{
	void init(VideoController& vcontroller, SDL_Renderer* renderer);
	void copyVRAMToBuffer(VideoController& vcontroller);
}