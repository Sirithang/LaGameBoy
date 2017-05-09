#pragma once

#include "gameboy.h"

const u8 ZERO_FLAG_BIT = 7;
const u8 SUBSTRACT_FLAG_BIT = 6;
const u8 HALF_CARRY_FLAG_BIT = 5;
const u8 CARRY_FLAG_BIT = 4;

//return 0 if bit is not 1 in value
#define BITTEST(value,bit) ((value) & (1<<(bit)))
#define BITSET(value,bit) value |= 1 << bit
#define BITCLEAR(value,bit) value &= ~(1 << bit)

#define HALF_CARRY_TEST_ADD(a,b) ((a&0xf) + (b&0xf))&0x10
#define HALF_CARRY_TEST_SUB(a,b) ((a&0xf) - (b&0xf))&0x10

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

	Addresser addresser;
};

namespace cpu
{
	int tick(CPU& cpu);
}