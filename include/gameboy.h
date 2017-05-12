#pragma once

#include "SDL.h"
#include "utils.h"
#include "cpu.h"
#include "graphic.h"

//Abstract all internal memory into a single struct for ease of access and tracking, even if some should be on their own system 
//(e.g. VRAM/OAM on GPU, IO register split onto each of the devices...)
struct InternalMemory
{ //all memeory size have +1, because the size have been computed as ending_adress - starting_adress. but ending adress is included.
	u8 VRAM[0x1FFF + 1];
	u8 OAM[0x9F + 1];
	u8 WRAM[0x1FFF + 1];
	u8 IORegister[0x7F + 1];
	u8 HRAM[0x7E + 1];
	u8 InterruptRegister;
};


struct Cart
{
	u8* content;
	//TODO handle MBC
};

struct Motherboard
{
	InternalMemory internalMemory;
	CPU cpu;
	GPU gpu;
	Cart cart;
};

namespace cart
{
	void load(Cart* cart, const char* path);
	u8* address(Cart* cart, u16 address);
}


namespace motherboard
{
	void init(Motherboard* motherboard);

	u8 fetchu8(Motherboard* motherboard, u16 address);
	s8 fetchs8(Motherboard* motherboard, u16 address);
	u16 fetchu16(Motherboard* motherboard, u16 address);

	//pointer version of the fetch, allow direct modification of data (no duplication)
	//mainly use by opcode helper macro
	u8* fetchu8p(Motherboard* controller, u16 address);

	void writeu8(Motherboard* motherboard, u16 address, u8 value);
	void writeu16(Motherboard* motherboard, u16 address, u16 value);

	void updateGPURegister(Motherboard* motherboard);
}