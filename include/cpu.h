#pragma once

#include "gameboy.h"

const u8 ZERO_FLAG_BIT = 7;
const u8 SUBSTRACT_FLAG_BIT = 6;
const u8 HALF_CARRY_FLAG_BIT = 5;
const u8 CARRY_FLAG_BIT = 4;


struct CPU
{
	struct Register
	{
		union
		{
			struct
			{
				u8 F:8;
				u8 A:8;
			};
			u16 AF;
		};

		union
		{
			struct
			{
				u8 C : 8;
				u8 B : 8;
			};
			u16 BC;
		};

		union
		{
			struct
			{
				u8 E : 8;
				u8 D : 8;
			};
			u16 DE;
		};

		union
		{
			struct
			{
				u8 L : 8;
				u8 H : 8;
			};
			u16 HL;
		};

	} registers;

	u16 SP;
	u16 PC;

	MemoryController memc;
};

namespace cpu
{
	int tick(CPU& cpu);
}