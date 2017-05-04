#include "cpu.h"
#include <stdio.h>

using namespace cpu;

#define BITTEST(value,bit) ((value) & (1<<(bit)))
#define BITSET(value,bit) value |= 1 << bit
#define BITCLEAR(value,bit) value &= ~(1 << bit)
#define HALF_CARRY_TEST(a,b) ((a&0xf) + (b&0xf))&0x10

void bitTest(CPU& cpu, u8 value, u8 bit)
{
	BITSET(cpu.registers.F, HALF_CARRY_FLAG_BIT);
	BITCLEAR(cpu.registers.F, SUBSTRACT_FLAG_BIT);

	BITTEST(value, bit) == 0 ? BITSET(cpu.registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu.registers.F, ZERO_FLAG_BIT);
}

int extendedDecode(CPU& cpu, u8 opcode)
{
	//printf("Decoding extended %hhX\n", opcode);

	switch (opcode)
	{
	case 0x7C: //BIT 7, H
		bitTest(cpu, cpu.registers.H, 7);
		break;
	default:
		printf("unknown extended opcode %hhX\n", opcode);
		return -1;
		break;
	}

	return 0;
}

int decode(CPU& cpu, u8 opcode)
{
	//printf("Decoding %hhX\n", opcode);
	switch (opcode)
	{
	case 0x0C:// INC C
		{
			BITCLEAR(cpu.registers.F, SUBSTRACT_FLAG_BIT);
			if (HALF_CARRY_TEST(cpu.registers.C, (u8)0x1) != 0)
				BITSET(cpu.registers.F, HALF_CARRY_FLAG_BIT);
			else
				BITCLEAR(cpu.registers.F, HALF_CARRY_FLAG_BIT);

			if (cpu.registers.C + (u8)0x1 == (u8)0x0)
				BITSET(cpu.registers.F, ZERO_FLAG_BIT);
			else
				BITCLEAR(cpu.registers.F, ZERO_FLAG_BIT);

			cpu.registers.C = cpu.registers.C + (u8)0x1;
		}
		break;
	case 0x0E:
		cpu.registers.C = memc::fetchu8(cpu.memc, cpu.PC);
		cpu.PC += 1;
		break;
	case 0x20://JN RZ, n (n is signed)
		if (BITTEST(cpu.registers.F, ZERO_FLAG_BIT) == 0)
			cpu.PC += memc::fetchs8(cpu.memc, cpu.PC) + 1; //+1, because we didn't advance PC after fetching offset.
		else
			cpu.PC += 1;
		break;
	case 0x21:
		{//LD HL,d16
			cpu.registers.HL = memc::fetchu16(cpu.memc, cpu.PC);
			cpu.PC += 2;
		}
		break;
	case 0x31:
		{//LD SP,d16
			cpu.SP = memc::fetchu16(cpu.memc, cpu.PC);
			cpu.PC += 2;
		}
		break;
	case 0x32:
		{
			memc::writeu8(cpu.memc, cpu.registers.HL, cpu.registers.A);
			cpu.registers.HL = cpu.registers.HL - 1;
		}
		break;
	case 0x3E:
		cpu.registers.A = memc::fetchu8(cpu.memc, cpu.PC);
		cpu.PC += 1;
		break;
	case 0xAF: //XOR A
		cpu.registers.A ^= cpu.registers.A;
		break;
	case 0xCB://extended opcode
		cpu.PC++;
		return extendedDecode(cpu, memc::fetchu8(cpu.memc, cpu.PC-1));
		break;
	case 0xE2:
		memc::writeu8(cpu.memc, 0xff00 + cpu.registers.C, cpu.registers.A);
		break;
	default:
		printf("unknown opcode %hhX\n", opcode);
		return -1;
		break;
	}

	return 0;
}

int cpu::tick(CPU& cpu)
{
	u8 opcode = memc::fetchu8(cpu.memc, cpu.PC);
	cpu.PC += 1;

	return decode(cpu, opcode);
}