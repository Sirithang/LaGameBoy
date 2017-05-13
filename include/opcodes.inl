#pragma once

#include "cpu.h"

inline void CPAn(CPU* cpu, u8 value)
{
	u8 result = cpu->registers.A - value;

	result == 0x0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
	BITSET(cpu->registers.F, SUBSTRACT_FLAG_BIT);
	HALF_CARRY_TEST_SUB(cpu->registers.A, value) != 0x0 ? BITSET(cpu->registers.F, HALF_CARRY_FLAG_BIT) : BITCLEAR(cpu->registers.F, HALF_CARRY_FLAG_BIT);

	cpu->registers.A < value ? BITSET(cpu->registers.F, CARRY_FLAG_BIT) : BITCLEAR(cpu->registers.F, CARRY_FLAG_BIT);
}

inline u8 INC8(CPU* cpu, u8 value)
{
	BITCLEAR(cpu->registers.F, SUBSTRACT_FLAG_BIT);
	HALF_CARRY_TEST_ADD(value, (u8)0x1) != 0 ? BITSET(cpu->registers.F, HALF_CARRY_FLAG_BIT) : BITCLEAR(cpu->registers.F, HALF_CARRY_FLAG_BIT);

	(u8)(value + 0x1) == (u8)0x0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);

	return value + 1;
}

inline u8 DEC8(CPU* cpu, u8 value)
{
	BITSET(cpu->registers.F, SUBSTRACT_FLAG_BIT);
	HALF_CARRY_TEST_SUB(value, (u8)0x1) != 0 ? BITSET(cpu->registers.F, HALF_CARRY_FLAG_BIT) : BITCLEAR(cpu->registers.F, HALF_CARRY_FLAG_BIT);

	(u8)(value - 0x1) == (u8)0x0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);

	return value - 1;
}

//do a + b
inline u8 ADD(CPU* cpu, u8 a, u8 b)
{
	BITCLEAR(cpu->registers.F, SUBSTRACT_FLAG_BIT);

	HALF_CARRY_TEST_ADD(a, b) != 0 ? BITSET(cpu->registers.F, HALF_CARRY_FLAG_BIT) : BITCLEAR(cpu->registers.F, HALF_CARRY_FLAG_BIT);
	(u8)(a + b) == (u8)0x0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);

	a + b > 0xFF ? BITSET(cpu->registers.F, CARRY_FLAG_BIT) : BITCLEAR(cpu->registers.F, CARRY_FLAG_BIT);

	return a + b;
}

//do a - b
inline u8 SUB(CPU* cpu, u8 a, u8 b)
{
	BITSET(cpu->registers.F, SUBSTRACT_FLAG_BIT);
	HALF_CARRY_TEST_SUB(a, b) != 0 ? BITSET(cpu->registers.F, HALF_CARRY_FLAG_BIT) : BITCLEAR(cpu->registers.F, HALF_CARRY_FLAG_BIT);
	a - b == (u8)0x0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
	a < b ? BITSET(cpu->registers.F, CARRY_FLAG_BIT) : BITCLEAR(cpu->registers.F, CARRY_FLAG_BIT);

	return a - b;
}

inline u8 SWAP(CPU* cpu, u8 value)
{
	u8 ret = ((value & 0x0F) << 4 | (value & 0xF0) >> 4);
	cpu->registers.F = 0;
	if (ret == 0) BITSET(cpu->registers.F, ZERO_FLAG_BIT);

	return ret;
}

inline u16 ADD(CPU* cpu, u16 a, u16 b)
{
	BITCLEAR(cpu->registers.F, SUBSTRACT_FLAG_BIT);
	HALF_CARRY_TEST_ADD_16(a, b) != 0 ? BITSET(cpu->registers.F, HALF_CARRY_FLAG_BIT) : BITCLEAR(cpu->registers.F, HALF_CARRY_FLAG_BIT);
	(u16)(a + b) == (u16)0x0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);

	a + b > 0xFFFF ? BITSET(cpu->registers.F, CARRY_FLAG_BIT) : BITCLEAR(cpu->registers.F, CARRY_FLAG_BIT);

	return a + b;
}

inline u8 RL(CPU* cpu, u8 value)
{
	u8 ret = value;

	ret = ret << 1;

	cpu->registers.F = 0;
	BITTEST(cpu->registers.F, CARRY_FLAG_BIT) == 0 ? BITCLEAR(ret, 0) : BITSET(ret, 0);
	BITTEST(value, 7) == 0 ? BITCLEAR(cpu->registers.F, CARRY_FLAG_BIT) : BITSET(cpu->registers.F, CARRY_FLAG_BIT);

	ret == (u8)0x0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);

	return ret;
}

inline u8 RLC(CPU* cpu, u8 value, u8 setZeroFlag)
{
	u8 ret = value;

	u8 bit = ret & (1<<7);
	ret = ret << 1;

	cpu->registers.F = 0;

	if (bit != 0)
	{
		BITSET(cpu->registers.F, CARRY_FLAG_BIT);
		BITSET(ret, 0);
	}

	if (setZeroFlag && ret == 0x0)
		BITSET(cpu->registers.F, ZERO_FLAG_BIT);

	return ret;
}

inline u8 RRC(CPU* cpu, u8 value, u8 setZeroFlag)
{
	u8 ret = value;

	u8 bit = ret & 0x1;
	ret = ret >> 1;

	cpu->registers.F = 0;

	if (bit != 0)
	{
		BITSET(cpu->registers.F, CARRY_FLAG_BIT);
		BITSET(ret, 7);
	}

	if (setZeroFlag && ret == 0x0)
		BITSET(cpu->registers.F, ZERO_FLAG_BIT);

	
	return ret;
}

inline u8 RES(CPU* cpu, u8 value, u8 bit)
{
	u8 ret = value;
	BITCLEAR(ret, bit);
	return ret;
}

inline u8 SRL(CPU* cpu, u8 value)
{
	u8 ret = value;
	cpu->registers.F = 0;
	if ((ret & 0x1) != 0) BITSET(cpu->registers.F, CARRY_FLAG_BIT);
	ret = ret >> 1;
	if (ret == 0x0) BITSET(cpu->registers.F, ZERO_FLAG_BIT);
	return ret;
}

inline u8 SET(CPU* cpu, u8 value, u8 bit)
{
	u8 ret = value;
	BITSET(ret, bit);
	return ret;
}

inline u8 SLA(CPU* cpu, u8 value)
{
	u8 ret = value;

	cpu->registers.F = 0;
	BITTEST(ret, 7) != 0x1 ? BITSET(cpu->registers.F, CARRY_FLAG_BIT) : BITCLEAR(cpu->registers.F, CARRY_FLAG_BIT);
	ret = ret << 1;
	if (ret == 0x0) BITSET(cpu->registers.F, ZERO_FLAG_BIT);
	return ret;
}

inline void BIT(CPU* cpu, u8 value, u8 bit)
{
	BITSET(cpu->registers.F, HALF_CARRY_FLAG_BIT);
	BITCLEAR(cpu->registers.F, SUBSTRACT_FLAG_BIT);

	BITTEST(value, bit) == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
}