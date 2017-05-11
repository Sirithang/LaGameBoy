#pragma once

#include "utils.h"

struct Motherboard;

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

	u8 interruptEnabled;

	Motherboard* mb;
};

namespace cpu
{
	int tick(CPU* cpu);
}