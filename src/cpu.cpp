#include "cpu.h"
#include "gameboy.h"
#include <stdio.h>

using namespace cpu;


inline void stackPush(CPU* cpu, u16 value)
{
	cpu->SP -= 2; //as we write in reverse, we need to decrease the stack point BEFORE writing
	motherboard::writeu16(cpu->mb, cpu->SP, value);
}

inline void stackPush(CPU* cpu, u8 value)
{
	cpu->SP -= 1;
	motherboard::writeu16(cpu->mb, cpu->SP, value);
}

inline u16 stackPop(CPU* cpu)
{
	cpu->SP += 2;//we should increase AFTER fetching, but to avoid stocking a temp var, we just offset by -2 we reading next
	return motherboard::fetchu16(cpu->mb, cpu->SP - 2);
}

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
	BITSET(cpu->registers.F, SUBSTRACT_FLAG_BIT);
	HALF_CARRY_TEST_ADD(value, (u8)0x1) != 0 ? BITSET(cpu->registers.F, HALF_CARRY_FLAG_BIT) : BITCLEAR(cpu->registers.F, HALF_CARRY_FLAG_BIT);

	value - (u8)0x1 == (u8)0x0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);

	return value + 1;
}

inline u8 DEC8(CPU* cpu, u8 value)
{
	BITSET(cpu->registers.F, SUBSTRACT_FLAG_BIT);
	HALF_CARRY_TEST_SUB(value, (u8)0x1) != 0 ? BITSET(cpu->registers.F, HALF_CARRY_FLAG_BIT) : BITCLEAR(cpu->registers.F, HALF_CARRY_FLAG_BIT);

	value - (u8)0x1 == (u8)0x0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);

	return value - 1;
}

//do a + b
inline u8 ADD(CPU* cpu, u8 a, u8 b)
{
	BITCLEAR(cpu->registers.F, SUBSTRACT_FLAG_BIT);
	HALF_CARRY_TEST_ADD(a, b) != 0 ? BITSET(cpu->registers.F, HALF_CARRY_FLAG_BIT) : BITCLEAR(cpu->registers.F, HALF_CARRY_FLAG_BIT);
	a + b == (u8)0x0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);

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

int extendedDecode(CPU* cpu, u8 opcode)
{
	//probably can do without, but too lazy to think about it
	u8 buffer;
	int cycle = -1;

	switch (opcode)
	{
	case 0x11://RL C (rotate left, carry goes to bit 0 and bit 7 goes to carry)
		buffer = BITTEST(cpu->registers.C, 7);
		cpu->registers.C = cpu->registers.C << 1;

		BITTEST(cpu->registers.F, CARRY_FLAG_BIT) == 0 ? BITCLEAR(cpu->registers.C, 0) : BITSET(cpu->registers.C, 0);
		buffer == 0 ? BITCLEAR(cpu->registers.F, CARRY_FLAG_BIT) : BITSET(cpu->registers.F, CARRY_FLAG_BIT);

		cpu->registers.C == (u8)0x0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		cycle = 8;
		break;
	case 0x7C: //BIT 7, H
		BITSET(cpu->registers.F, HALF_CARRY_FLAG_BIT);
		BITCLEAR(cpu->registers.F, SUBSTRACT_FLAG_BIT);

		BITTEST(cpu->registers.H, 7) == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		cycle = 8;
		break;
	default:
		printf("unknown extended opcode %hhX at address %hX \n", opcode, cpu->PC - 1);
		break;
	}

	return cycle;
}

int decode(CPU* cpu, u8 opcode)
{
	//probably can do without, but too lazy to think about it
	u8 buffer;

	int cycle = -1;

	switch (opcode)
	{
	case 0x04:// INC B
		{
			cpu->registers.B = INC8(cpu, cpu->registers.B);
			cycle = 4;
		}
		break;
	case 0x05:// DEC B
		{
			cpu->registers.B = DEC8(cpu, cpu->registers.B);
			cycle = 4;
		}
		break;
	case 0x06: // LD B, n
		cpu->registers.B = motherboard::fetchu8(cpu->mb, cpu->PC);
		cpu->PC += 1;
		cycle = 8;
		break;
	case 0x0C:// INC C
		cpu->registers.C = INC8(cpu, cpu->registers.C);
		cycle = 4;
		break;
	case 0x0D:// DEC C
		cpu->registers.C = DEC8(cpu, cpu->registers.C);
		cycle = 4;
		break;
	case 0x0E: // LD C, d8
		cpu->registers.C = motherboard::fetchu8(cpu->mb, cpu->PC);
		cpu->PC += 1;
		cycle = 8;
		break;
	case 0x11://LD DE, d16
		cpu->registers.DE = motherboard::fetchu16(cpu->mb, cpu->PC);
		cpu->PC += 2;
		cycle = 12;
		break;
	case 0x13: //INC DE
		cpu->registers.DE += 1;
		cycle = 8;
		break;
	case 0x14: //INC D
		cpu->registers.D = DEC8(cpu, cpu->registers.D);
		cycle = 4;
		break;
	case 0x15: //DEC D
		cpu->registers.D = DEC8(cpu, cpu->registers.D);
		cycle = 4;
		break;
	case 0x16: //LD D,d8
		cpu->registers.D = motherboard::fetchu8(cpu->mb, cpu->PC);
		cpu->PC += 1;
		cycle = 8;
		break;
	case 0x17://RL A (rotate left, carry goes to bit 0 and bit 7 goes to carry)
		buffer = BITTEST(cpu->registers.A, 7);
		cpu->registers.A = cpu->registers.A << 1;

		BITTEST(cpu->registers.F, CARRY_FLAG_BIT) == 0 ? BITCLEAR(cpu->registers.A, 0) : BITSET(cpu->registers.A, 0);
		buffer == 0 ? BITCLEAR(cpu->registers.F, CARRY_FLAG_BIT) : BITSET(cpu->registers.F, CARRY_FLAG_BIT);
		cycle = 4;
		break;
	case 0x18: //JR r8 (r == relative, it is signed)
		cpu->PC += motherboard::fetchs8(cpu->mb, cpu->PC) + 1;
		cycle = 12;
		break;
	case 0x1A: //LD A, (DE)
		cpu->registers.A = motherboard::fetchu8(cpu->mb, cpu->registers.DE);
		cycle = 8;
		break;
	case 0x1C://INC E
		cpu->registers.E = INC8(cpu, cpu->registers.E);
		cycle = 4;
		break;
	case 0x1D://DEC E
		cpu->registers.E = DEC8(cpu, cpu->registers.E);
		cycle = 4;
		break;
	case 0x1E: //LD E,d8
		cpu->registers.E = motherboard::fetchu8(cpu->mb, cpu->PC);
		cpu->PC += 1;
		cycle = 8;
		break;
	case 0x20://JR NZ, n (n is signed)
		if (BITTEST(cpu->registers.F, ZERO_FLAG_BIT) == 0)
		{
			cpu->PC += motherboard::fetchs8(cpu->mb, cpu->PC) + 1; //+1, because we didn't advance PC after fetching offset.
			cycle = 12;
		}
		else
		{
			cpu->PC += 1;
			cycle = 8;
		}
		break;
	case 0x21:
		{//LD HL,d16
			cpu->registers.HL = motherboard::fetchu16(cpu->mb, cpu->PC);
			cpu->PC += 2;
			cycle = 12;
		}
		break;
	case 0x22: //LD (HL+), A
		motherboard::writeu8(cpu->mb, cpu->registers.HL, cpu->registers.A);
		cpu->registers.HL += 1;
		cycle = 8;
		break;
	case 0x23: // INC HL
		cpu->registers.HL += 1;
		cycle = 8;
		break;
	case 0x24://INC H
		cpu->registers.H = INC8(cpu, cpu->registers.H);
		cycle = 4;
		break;
	case 0x25://DEC H
		cpu->registers.H = DEC8(cpu, cpu->registers.H);
		cycle = 4;
		break;
	case 0x28://JR Z, n (n is signed)
		if (BITTEST(cpu->registers.F, ZERO_FLAG_BIT) != 0)
		{
			cpu->PC += motherboard::fetchs8(cpu->mb, cpu->PC) + 1; //+1, because we didn't advance PC after fetching offset.
			cycle = 12;
		}
		else 
		{
			cpu->PC += 1;
			cycle = 8;
		}
		break;
	case 0x2E:// LD L, d8
		cpu->registers.L = motherboard::fetchu8(cpu->mb, cpu->PC);
		cpu->PC += 1;
		cycle = 8;
		break;
	case 0x31:
		{//LD SP,d16
			cpu->SP = motherboard::fetchu16(cpu->mb, cpu->PC);
			cpu->PC += 2;
			cycle = 12;
		}
		break;
	case 0x32: //LD (HL-), A
		{
			motherboard::writeu8(cpu->mb, cpu->registers.HL, cpu->registers.A);
			cpu->registers.HL = cpu->registers.HL - 1;
			cycle = 8;
		}
		break;
	case 0x3C: //INC A
		cpu->registers.A = INC8(cpu, cpu->registers.A);
		cycle = 4;
		break;
	case 0x3D: //DEC A
		cpu->registers.A = DEC8(cpu, cpu->registers.A);
		cycle = 4;
		break;
	case 0x3E: //LD A, d8
		cpu->registers.A = motherboard::fetchu8(cpu->mb, cpu->PC);
		cpu->PC += 1;
		cycle = 8;
		break;
	case 0x4F : //LD C, A
		cpu->registers.C = cpu->registers.A;
		cycle = 4;
		break;
	case 0x57://LD D,A
		cpu->registers.D = cpu->registers.A;
		cycle = 4;
		break;
	case 0x67://LD H, A
		cpu->registers.H = cpu->registers.A;
		cycle = 4;
		break;
	case 0x77: //LD (HL),A
		motherboard::writeu8(cpu->mb, cpu->registers.HL, cpu->registers.A);
		cycle = 8;
		break;
	case 0x78: //LD A,B
		cpu->registers.A = cpu->registers.B;
		cycle = 4;
		break;
	case 0x79: //LD A,C
		cpu->registers.A = cpu->registers.C;
		cycle = 4;
		break;
	case 0x7A: //LD A,D
		cpu->registers.A = cpu->registers.D;
		cycle = 4;
		break;
	case 0x7B: //LD A, E
		cpu->registers.A = cpu->registers.E;
		cycle = 4;
		break;
	case 0x7C: //LD A,H
		cpu->registers.A = cpu->registers.H;
		cycle = 4;
		break;
	case 0x7D : //LD A,L
		cpu->registers.A = cpu->registers.L;
		cycle = 4;
		break;
	case 0x86: //ADD A,(HL)
		cpu->registers.A = ADD(cpu, cpu->registers.A, motherboard::fetchu8(cpu->mb, cpu->registers.HL));
		cycle = 8;
		break;
	case 0x90: // SUB B
		cpu->registers.A = SUB(cpu, cpu->registers.A, cpu->registers.B);
		cycle = 4;
		break;
	case 0xAF: //XOR A
		cpu->registers.A ^= cpu->registers.A;
		cpu->registers.A == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		cycle = 4;
		break;
	case 0xBE: //CP (HL)
		CPAn(cpu, motherboard::fetchu8(cpu->mb, cpu->registers.HL));
		cycle = 8;
		break;
	case 0xC1: //pop BC
		cpu->registers.BC = stackPop(cpu);
		cycle = 12;
		break;
	case 0xC5://push BC
		stackPush(cpu, cpu->registers.BC);
		cycle = 16;
		break;
	case 0xC9:// RET
		cpu->PC = stackPop(cpu);
		cycle = 16;
		break;
	case 0xCB://extended opcode
		cpu->PC++;
		return extendedDecode(cpu, motherboard::fetchu8(cpu->mb, cpu->PC-1));
		break;
	case 0xCD: //CALL a16
		//we offset the PC by 2, because there is the 16bit address befor ethe next instruction
		stackPush(cpu, (u16)(cpu->PC+2));
		cpu->PC = motherboard::fetchu16(cpu->mb, cpu->PC);
		cycle = 24;
		break;
	case 0xE0: //LDH (a8), A
		motherboard::writeu8(cpu->mb, 0xff00 + motherboard::fetchu8(cpu->mb, cpu->PC), cpu->registers.A);
		cpu->PC += 1;
		cycle = 12;
		break;
	case 0xE2:// LD (C), A
		motherboard::writeu8(cpu->mb, 0xff00 + cpu->registers.C, cpu->registers.A);
		cycle = 12;
		break;
	case 0xEA: //LD (a16), A
		motherboard::writeu8(cpu->mb, motherboard::fetchu16(cpu->mb, cpu->PC), cpu->registers.A);
		cpu->PC += 2;
		cycle = 16;
		break;
	case 0xF0: // LDH A,(a8)
		cpu->registers.A = motherboard::fetchu8(cpu->mb, 0xFF00 + motherboard::fetchu8(cpu->mb, cpu->PC));
		cpu->PC += 1;
		cycle = 12;
		break;
	case 0xFE:// CP d8
		CPAn(cpu, motherboard::fetchu8(cpu->mb, cpu->PC));
		cpu->PC += 1;
		cycle = 8;
		break;
	default:
		printf("unknown opcode %hhX at address %hX \n", opcode, cpu->PC - 1);
		break;
	}

	return cycle;
}

int cpu::tick(CPU* cpu)
{
	if (cpu->PC == 0xf9)
		printf("break");

	u8 opcode = motherboard::fetchu8(cpu->mb, cpu->PC);
	cpu->PC += 1;

	return decode(cpu, opcode);
}