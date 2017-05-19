#include "gameboy.h"
#include "graphic.h"
#include <stdio.h>

#define CAST_MEMPTR_TO_SHORT(val) *(u16*)(val)
#define FETCH_CAST(val) length > 1?

using namespace cart;
using namespace motherboard;

static u8 g_bootstrap[] = 
{
	49, 254, 255, 175, 33, 255, 159, 50, 203, 124, 32, 251, 33, 38, 255, 14, 17, 62, 128, 50, 226, 12, 62, 243, 226, 50, 62, 119, 119, 62, 252, 224, 71, 17, 4, 1, 33, 16, 128, 26, 205, 149, 0, 205, 150, 0, 19, 123, 254, 52, 32, 243, 17, 216, 0, 6, 8, 26, 19, 34, 35,
	5, 32, 249, 62, 25, 234, 16, 153, 33, 47, 153, 14, 12, 61, 40, 8, 50, 13, 32, 249, 46, 15, 24, 243, 103, 62, 100, 87, 224, 66, 62, 145, 224, 64, 4, 30, 2, 14, 12, 240, 68, 254, 144, 32, 250, 13, 32, 247, 29, 32, 242, 14, 19, 36, 124, 30, 131, 254, 98, 40, 6, 30,
	193, 254, 100, 32, 6, 123, 226, 12, 62, 135, 226, 240, 66, 144, 224, 66, 21, 32, 210, 5, 32, 79, 22, 32, 24, 203, 79, 6, 4, 197, 203, 17, 23, 193, 203, 17, 23, 5, 32, 245, 34, 35, 34, 35, 201, 206, 237, 102, 102, 204, 13, 0, 11, 3, 115, 0, 131, 0, 12, 0, 13, 0, 8,
	17, 31, 136, 137, 0, 14, 220, 204, 110, 230, 221, 221, 217, 153, 187, 187, 103, 99, 110, 14, 236, 204, 221, 220, 153, 159, 187, 185, 51, 62, 60, 66, 185, 165, 185, 165, 66, 60, 33, 4, 1, 17, 168, 0, 26, 19, 190, 32, 254, 35, 125, 254, 52, 32, 245, 6, 25, 120,
	134, 35, 5, 32, 251, 134, 32, 254, 62, 1, 224, 80, 0
};

//used to allow writing to "forbidden" place, as some game do write on forbidden place...
u16 dummyMemory;
u8 dummy8bit;

//Last parameters is input when writing and output when fetching.
//THAT FONCTION WON'T WRITE TO MEMORY. You still need after fetching the pointer to write yourself
//It will however use written value for special case (MBC banck switch, ioRegister etc..) & return pointer to a
//dummy discarded memory if the written address is a read only space
inline void fetchMemory(Motherboard* motherboard, u16 address, u8 writing, u8** value)
{	
	if (address <= 0xFF)
	{
		if(motherboard->internalMemory.IORegister[0x50])
			cart::address(&motherboard->cart, address, writing, value);
		else
			*value = &g_bootstrap[address];		
	}
	else if(address <= 0x7FFF)
	{
		cart::address(&motherboard->cart, address, writing, value);
	}
	else if(address <= 0x9FFF)
	{
		*value = &motherboard->internalMemory.VRAM[address - 0x8000];
	}
	else if (address <= 0xBFFF)
	{
		cart::address(&motherboard->cart, address, writing, value);
	}
	else if (address <= 0xDFFF)
	{
		*value = &motherboard->internalMemory.WRAM[address - 0xC000];
	}
	else if (address <= 0xFDFF)
	{//ECHO ram
		//TODO : handle echo ram?
		*value = (u8*)&dummyMemory;
	}
	else if (address <= 0xFE9F)
	{
		*value = &motherboard->internalMemory.OAM[address - 0xFE00];
	}
	else if (address <= 0xFEFF)
	{
		*value = (u8*)&dummyMemory;
	}
	else if (address <= 0xFF7F)
	{
		internalmemory::ioRegisterAccess(&motherboard->internalMemory, address, writing, value);
	}
	else if (address <= 0xFFFE)
	{
		*value = &motherboard->internalMemory.HRAM[address - 0xFF80];
	}
	else if(address == 0xFFFF)
	{
		*value = &motherboard->internalMemory.InterruptRegister;
	}
	else
	{
		printf("Bad asked memory %hx\n", address);
		*value = 0;
	}
}

//== cart


void cart::load(Cart* cart, const char* path)
{
	FILE *file;
	unsigned long fileLen;

	//Open file
	fopen_s(&file, path, "rb");
	if (!file)
	{
		fprintf(stderr, "Unable to open file %s\n", path);
		return;
	}

	//Get file length
	fseek(file, 0, SEEK_END);
	fileLen = ftell(file);
	fseek(file, 0, SEEK_SET);

	//Allocate memory
	cart->content = (u8*)malloc(fileLen + 1);
	if (!cart->content)
	{
		fprintf(stderr, "Memory error!");
		fclose(file);
		return;
	}

	//Read file contents into buffer
	fread(cart->content, fileLen, 1, file);
	fclose(file);

	cart->MBCType = cart->content[0x147];
	cart->ROMBankNumber = 0x01;

	u8 ramSize = cart->content[0x149];
	switch (ramSize)
	{
	case 0x01:
		cart->RAMSize = 2000;
		break;
	case 0x02:
		cart->RAMSize = 8000;
		break;
	case 0x03:
		cart->RAMSize = 32000;
		break;
	default:
		cart->RAMSize = 0;
		break;
	}

	cart->RAMBank = 0;
	cart->RAM = cart->RAMSize == 0 ? NULL : new u8[cart->RAMSize];
}

void cart::address(Cart* cart, u16 address, u8 write, u8** value)
{
	if (write != 0)
	{
		if (address < 0x2000)
		{
			//TODO : enable RAM? (prob useless as it was for hardware reason you ahd to enable/disbale it before/after writing
		}
		else if (address < 0x4000)
		{
			u8 val = (**value & 0x1F);

			if (val == 0) val = 1;

			//select only 5 lowest bit 
			cart->ROMBankNumber = (cart->ROMBankNumber & 0xE0) + val;
			*value = &dummy8bit;
		}
		else if (address < 0x6000)
		{
			//either upper 3 bit of bank number or RAM bank number depending on mode selected
			if (cart->modeROMRAM == 0)
			{//rom mode
				u8 val = (**value & 0xE0);
				cart->ROMBankNumber = (cart->ROMBankNumber & 0x1F) + val;
			}
			else
			{//ram mode
				cart->RAMBank = SDL_min(**value, 3);
			}
		}
		else if (address < 0x8000)
		{
			//selecting ram or rom mode
			cart->modeROMRAM = (**value) & 0x1;
		}
		else
		{
			printf("Unhandled writing to Cart ROM address %hx\n", address);
		}
		*value = &dummy8bit;
	}
	else
	{
		if(address < 0x4000 || cart->MBCType == 0)
			*value = cart->content + address;
		else
		{
			if (address < 0x8000)
			{
				int bankNum = cart->ROMBankNumber == 0 ? 1 : cart->ROMBankNumber;
				*value = cart->content + (address + (cart->ROMBankNumber - 1) * 0x4000);
			}
			else if (address >= 0xA000 && address <= 0xBFFF && cart->RAMSize > 0)
			{
				*value = cart->RAM + (address - 0xA000 + (cart->RAMBank - 1) * 0x2000);
			}
			else
			{
				printf("Trying to read from cart from a wrong address %hx\n", address);
				*value = (u8*)&dummyMemory;
			}
		}
	}
}

//=======

int counter = 0;
int up = 0;

void sqr_waveGen(void *_sqrWave, Uint8 *_stream, int _length)
{
	for (int i = 0; i < _length; ++i)
	{
		_stream[i] = up ? 255 : 0;

		counter++;
		if (counter > 256)
		{
			counter = 0;
			up = 1 - up;
		}
	}
}

void soundgenerator::init(SoundGenerator* generator)
{
	SDL_AudioSpec spec;

	SDL_zero(spec);

	spec.freq = 480000;
	spec.format = AUDIO_U8;
	spec.channels = 1;
	spec.samples = 16;
	spec.callback = sqr_waveGen;
	spec.userdata = &generator->sqrWave0;

	SDL_AudioSpec out;

	generator->sqrWave0.deviceID = SDL_OpenAudioDevice(NULL, 0, &spec, &out, 0);

	if (generator->sqrWave0.deviceID == 0)
	{
		SDL_Log("Failed to open audio: %s", SDL_GetError());
	}

	SDL_PauseAudioDevice(generator->sqrWave0.deviceID, 1);
}

//=======

void motherboard::init(Motherboard* motherboard)
{
	motherboard->cpu.mb = motherboard;
	motherboard->gpu.mb = motherboard;
	motherboard->internalMemory.mb = motherboard;

	motherboard->cpu.dividerCounter = 0;
	motherboard->cpu.timerCycleCounter = 0;
	motherboard->cpu.halted = 0;

	//enable DMG rom
	motherboard->internalMemory.IORegister[0x50] = 0;

	SDL_zero(motherboard->internalMemory.IORegister);

	motherboard->inputState = 0xFF;
	motherboard->DMARequested = 0;

	soundgenerator::init(&motherboard->sound);
}

u8 motherboard::fetchu8(Motherboard* controller, u16 address)
{
	u8* ret;
	fetchMemory(controller, address, 0, &ret);
	return *ret;
}

u8* motherboard::fetchu8p(Motherboard* controller, u16 address)
{
	u8* ret;
	fetchMemory(controller, address, 0, &ret);
	return ret;
}


s8 motherboard::fetchs8(Motherboard* controller, u16 address)
{
	u8* ret;
	fetchMemory(controller, address, 0, &ret);
	return *ret;
}

u16 motherboard::fetchu16(Motherboard* controller, u16 address)
{
	u8* ret;
	fetchMemory(controller, address, 0, &ret);
	return CAST_MEMPTR_TO_SHORT(ret);
}

void motherboard::writeu8(Motherboard* controller, u16 address, u8 value)
{
	u8* pos = &value;
	fetchMemory(controller, address, 1, &pos);
	//the fetch mem does not write the value to memory (unless for specific write that trigger thing)
	//so we write to whatever pointer we've been fed back 
	*pos = value;
}

void motherboard::writeu16(Motherboard* controller, u16 address, u16 value)
{
	u16* pos = &value;
	fetchMemory(controller, address, 1, (u8**)&pos);
	*pos = value;
}

void motherboard::updateGPURegister(Motherboard* motherboard)
{
	motherboard->internalMemory.IORegister[0x44] =  motherboard->gpu.currentLine;

	//-- setup LCDC Status register data
	u8 val = motherboard->internalMemory.IORegister[0x41];

	//set the 2 last bit to 0, then OR them with the state to store state in last 2 bit
	val &= ~(val & 0x3);
	val |= motherboard->gpu.currentState;

	if (motherboard->gpu.currentLine == motherboard->internalMemory.IORegister[0x45])
	{
		BITSET(val, 2);

		if (BITTEST(motherboard->internalMemory.IORegister[0x41], 6) != 0x0)
		{//STAT ask for coincidence interrupt
			BITSET(motherboard->internalMemory.IORegister[0x0F], 1);
		}

	}
	else
	{
		BITCLEAR(val, 2);
	}


	motherboard->internalMemory.IORegister[0x41] = val;
	//--


}

//==================================

void internalmemory::ioRegisterAccess(InternalMemory* intmem, u16 address, u8 write, u8** value)
{
	switch (address)
	{
	case 0xFF00://INPUT
		if (write)
		{//we let write even on readonly, as it won't change any value anyway
			*value = &intmem->IORegister[0x00];
		}
		else
		{//on read, we return the right value depending on what was asked
			if (BITTEST(intmem->IORegister[0x00], 4) == 0)
			{//asked for direction
				dummy8bit = intmem->mb->inputState >> 4;
			}
			else
			{//let's assume it's the other one? We need to return soemthing anyway
			 // if a game get random input even when not pressed, will be to look for I suppose
				dummy8bit = intmem->mb->inputState & 0x0f;
			}
			*value = &dummy8bit;
		}
		break;

	case 0xFF04 : //Divider Register. get 1 every 256 cycle. Writing reset it
		if (write)
		{
			intmem->IORegister[0x04] = 0;
			*value = &dummy8bit;
		}
		else
			*value = &intmem->IORegister[0x04];
		break;

	//all those are R/W so no special handling to do
	case 0xFF01:// some cable link stuff, handle WAYYY later
	case 0xFF02:// some cable link stuff, handle WAYYY later

	case 0xFF07:
	case 0xFF06:
	case 0xFF10:
	case 0xFF11:
	case 0xFF12:
	case 0xFF13:
	case 0xFF14:
	case 0xFF16:
	case 0xFF17:
	case 0xFF18:
	case 0xFF19:
	case 0xFF1A:
	case 0xFF1B:
	case 0xFF1C:
	case 0xFF1D:
	case 0xFF1E:
	case 0xFF20:
	case 0xFF21:
	case 0xFF22:
	case 0xFF23:
	case 0xFF24:
	case 0xFF25: // this should be accessible only in write, but let's handle read instead of returning garbage
	case 0xFF26: // TODO : maybe handle resetting content of all sound when bit 7 == 0

	case 0xFF30:
	case 0xFF31:
	case 0xFF32:
	case 0xFF33:
	case 0xFF34:
	case 0xFF35:
	case 0xFF36:
	case 0xFF37:
	case 0xFF38:
	case 0xFF39:
	case 0xFF3A:
	case 0xFF3B:
	case 0xFF3C:
	case 0xFF3D:
	case 0xFF3E:
	case 0xFF3F:
	case 0xFF40:
	case 0xFF41: //some bit a read only but hard to enforce wo/ more work, let's hop it don't break ¯\_(?)_/¯
	case 0xFF42: 
	case 0xFF43:
	case 0xFF45:
	case 0xFF47:
	case 0xFF48:
	case 0xFF49:
	case 0xFF4a:
	case 0xFF4b:
	case 0xFF50:
	case 0xFF0F:
	case 0xFF7F://?? No idea what that is 
		*value = &intmem->IORegister[address-0xff00];
		break;
	case 0xFF44:
		if (write)
		{//writing to the LCDC Y-Coord reset it
			intmem->IORegister[0x44] = 0;
			*value = &dummy8bit;
		}
		else //reading work as normal
			*value = &intmem->IORegister[0x44];
		break;
	case 0xFF46:// DMA transfer required
		*value = &dummy8bit; //this in case we try to read (shouldn't be allowed)
		if (write)
		{//let the program write to it, the DMA will use it
			//tell the motherboard a DMA transfer was required
			intmem->mb->DMARequested = 1;
			*value = &intmem->IORegister[0x46];
		}
		break;
	default:
		//if pass through it was an invalide read or write
		//so return a dummy value use as buffer
		*value = (u8*)&dummyMemory;
		printf("Unhandled IO register %hx\n", address);
		break;
	}
}