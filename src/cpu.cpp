#include "cpu.h"
#include <stdio.h>

using namespace cpu;

//return 0 if bit is not 1 in value
#define BITTEST(value,bit) ((value) & (1<<(bit)))
#define BITSET(value,bit) value |= 1 << bit
#define BITCLEAR(value,bit) value &= ~(1 << bit)
#define HALF_CARRY_TEST(a,b) ((a&0xf) + (b&0xf))&0x10

inline void stackPush(CPU& cpu, u16 value)
{
	addresser::writeu16(cpu.addresser, cpu.SP, value);
	cpu.SP += 1;
}

inline void stackPush(CPU& cpu, u8 value)
{
	addresser::writeu16(cpu.addresser, cpu.SP, value);
	cpu.SP += 1;
}

int extendedDecode(CPU& cpu, u8 opcode)
{
	//probably can do without, but too lazy to think about it
	u8 buffer;
	//printf("Decoding extended %hhX\n", opcode);

	switch (opcode)
	{
	case 0x11://RL C (rotate left, carry goes to bit 0 and bit 7 goes to carry)
		buffer = BITTEST(cpu.registers.C, 7);
		cpu.registers.C = cpu.registers.C << 1;

		BITTEST(cpu.registers.F, CARRY_FLAG_BIT) == 0 ? BITCLEAR(cpu.registers.C, 0) : BITSET(cpu.registers.C, 0);
		buffer == 0 ? BITCLEAR(cpu.registers.F, CARRY_FLAG_BIT) : BITSET(cpu.registers.F, CARRY_FLAG_BIT);

		cpu.registers.C == (u8)0x0 ? BITSET(cpu.registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu.registers.F, ZERO_FLAG_BIT);
		break;
	case 0x7C: //BIT 7, H
		BITSET(cpu.registers.F, HALF_CARRY_FLAG_BIT);
		BITCLEAR(cpu.registers.F, SUBSTRACT_FLAG_BIT);

		BITTEST(cpu.registers.H, 7) == 0 ? BITSET(cpu.registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu.registers.F, ZERO_FLAG_BIT);
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
	case 0x06: // LD B, n
		cpu.registers.B = addresser::fetchu8(cpu.addresser, cpu.PC);
		cpu.PC += 1;
		break;
	case 0x04:// INC B
		{
			BITCLEAR(cpu.registers.F, SUBSTRACT_FLAG_BIT);
			if (HALF_CARRY_TEST(cpu.registers.B, (u8)0x1) != 0)
				BITSET(cpu.registers.F, HALF_CARRY_FLAG_BIT);
			else
				BITCLEAR(cpu.registers.F, HALF_CARRY_FLAG_BIT);

			if (cpu.registers.B + (u8)0x1 == (u8)0x0)
				BITSET(cpu.registers.F, ZERO_FLAG_BIT);
			else
				BITCLEAR(cpu.registers.F, ZERO_FLAG_BIT);

			cpu.registers.B = cpu.registers.B + (u8)0x1;
		}
		break;
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
		cpu.registers.C = addresser::fetchu8(cpu.addresser, cpu.PC);
		cpu.PC += 1;
		break;
	case 0x11:
		cpu.registers.DE = addresser::fetchu16(cpu.addresser, cpu.PC);
		cpu.PC += 2;
		break;
	case 0x1A:
		cpu.registers.A = addresser::fetchu8(cpu.addresser, cpu.registers.DE);
		break;
	case 0x20://JN RZ, n (n is signed)
		if (BITTEST(cpu.registers.F, ZERO_FLAG_BIT) == 0)
			cpu.PC += addresser::fetchs8(cpu.addresser, cpu.PC) + 1; //+1, because we didn't advance PC after fetching offset.
		else
			cpu.PC += 1;
		break;
	case 0x21:
		{//LD HL,d16
			cpu.registers.HL = addresser::fetchu16(cpu.addresser, cpu.PC);
			cpu.PC += 2;
		}
		break;
	case 0x31:
		{//LD SP,d16
			cpu.SP = addresser::fetchu16(cpu.addresser, cpu.PC);
			cpu.PC += 2;
		}
		break;
	case 0x32:
		{
			addresser::writeu8(cpu.addresser, cpu.registers.HL, cpu.registers.A);
			cpu.registers.HL = cpu.registers.HL - 1;
		}
		break;
	case 0x3E:
		cpu.registers.A = addresser::fetchu8(cpu.addresser, cpu.PC);
		cpu.PC += 1;
		break;
	case 0x4F : //LD C, A
		cpu.registers.C = cpu.registers.A;
		break;
	case 0x77: //LD (HL),A
		addresser::writeu8(cpu.addresser, cpu.registers.HL, cpu.registers.A);
		break;
	case 0xAF: //XOR A
		cpu.registers.A ^= cpu.registers.A;
		break;
	case 0xC5://push BC
		stackPush(cpu, cpu.registers.BC);
		break;
	case 0xCB://extended opcode
		cpu.PC++;
		return extendedDecode(cpu, addresser::fetchu8(cpu.addresser, cpu.PC-1));
		break;
	case 0xCD: //CALL a16
		addresser::writeu16(cpu.addresser, cpu.SP, cpu.PC);
		cpu.PC = addresser::fetchu16(cpu.addresser, cpu.PC);
		break;
	case 0xE0:
		addresser::writeu8(cpu.addresser, 0xff00 + addresser::fetchu8(cpu.addresser, cpu.PC), cpu.registers.A);
		cpu.PC += 1;
		break;
	case 0xE2:
		addresser::writeu8(cpu.addresser, 0xff00 + cpu.registers.C, cpu.registers.A);
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
	u8 opcode = addresser::fetchu8(cpu.addresser, cpu.PC);
	cpu.PC += 1;

	return decode(cpu, opcode);
}