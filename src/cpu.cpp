#include "cpu.h"
#include <stdio.h>

using namespace cpu;

#define BITTEST(value,bit) (value >> bit) & 1
#define BITSET(value,bit) value |= 1 << bit
#define BITCLEAR(value,bit) value &= ~(1 << bit)

void bitTest(CPU& cpu, u8 value, u8 bit)
{
	BITSET(cpu.registers.F, HALF_CARRY_FLAG_BIT);
	BITCLEAR(cpu.registers.F, SUBSTRACT_FLAG_BIT);

	BITTEST(value, bit) == 0 ? BITSET(cpu.registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu.registers.F, ZERO_FLAG_BIT);
}

int extendedDecode(CPU& cpu, u8 opcode)
{
	printf("Decoding extended %hhX\n", opcode);

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
	printf("Decoding %hhX\n", opcode);
	switch (opcode)
	{
	case 0x20://JN RZ, n
		printf("jump by %hhX\n", memc::fetchu8(cpu.memc, cpu.PC));
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
	case 0xAF: //XOR A
		cpu.registers.A ^= cpu.registers.A;
		break;
	case 0xCB://extended opcode
		cpu.PC++;
		return extendedDecode(cpu, memc::fetchu8(cpu.memc, cpu.PC-1));
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