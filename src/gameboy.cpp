#include "gameboy.h"

#define CAST_MEMPTR_TO_SHORT(val) *(u16*)(val)

using namespace memc;
using namespace videocontroller;

static u8 g_bootstrap[] = 
{
	49, 254, 255, 175, 33, 255, 159, 50, 203, 124, 32, 251, 33, 38, 255, 14, 17, 62, 128, 50, 226, 12, 62, 243, 226, 50, 62, 119, 119, 62, 252, 224, 71, 17, 4, 1, 33, 16, 128, 26, 205, 149, 0, 205, 150, 0, 19, 123, 254, 52, 32, 243, 17, 216, 0, 6, 8, 26, 19, 34, 35,
	5, 32, 249, 62, 25, 234, 16, 153, 33, 47, 153, 14, 12, 61, 40, 8, 50, 13, 32, 249, 46, 15, 24, 243, 103, 62, 100, 87, 224, 66, 62, 145, 224, 64, 4, 30, 2, 14, 12, 240, 68, 254, 144, 32, 250, 13, 32, 247, 29, 32, 242, 14, 19, 36, 124, 30, 131, 254, 98, 40, 6, 30,
	193, 254, 100, 32, 6, 123, 226, 12, 62, 135, 226, 240, 66, 144, 224, 66, 21, 32, 210, 5, 32, 79, 22, 32, 24, 203, 79, 6, 4, 197, 203, 17, 23, 193, 203, 17, 23, 5, 32, 245, 34, 35, 34, 35, 201, 206, 237, 102, 102, 204, 13, 0, 11, 3, 115, 0, 131, 0, 12, 0, 13, 0, 8,
	17, 31, 136, 137, 0, 14, 220, 204, 110, 230, 221, 221, 217, 153, 187, 187, 103, 99, 110, 14, 236, 204, 221, 220, 153, 159, 187, 185, 51, 62, 60, 66, 185, 165, 185, 165, 66, 60, 33, 4, 1, 17, 168, 0, 26, 19, 190, 32, 254, 35, 125, 254, 52, 32, 245, 6, 25, 120,
	134, 35, 5, 32, 251, 134, 32, 254, 62, 1, 224, 80, 0
};

inline u8* fetchMemory(MemoryController& controller, u16 address)
{
	if (address <= 0xFF)
	{
		return &g_bootstrap[address];
	}
	else if (address >= 0x8000 && address <= 0x9FFF)
	{
		return (controller.vcontroller.VRAM + (address - 0x8000));
	}

	return 0;
}

u8 memc::fetchu8(MemoryController& controller, u16 address)
{
	return *fetchMemory(controller, address);
}

u16 memc::fetchu16(MemoryController& controller, u16 address)
{
	return CAST_MEMPTR_TO_SHORT(fetchMemory(controller, address));
}

void memc::writeu8(MemoryController& controller, u16 address, u8 value)
{
	*(fetchMemory(controller, address)) = value;
}

void memc::writeu16(MemoryController& controller, u16 address, u16 value)
{
	*(u16*)(fetchMemory(controller, address)) = value;
}

//==================================

void videocontroller::init(VideoController& vcontroller, SDL_Renderer* renderer)
{
	vcontroller.displayBuffer = SDL_CreateTexture(renderer,
		SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, SCREEN_WIDTH, SCREEN_HEIGHT);
}

void videocontroller::copyVRAMToBuffer(VideoController& vcontroller)
{

}