#include "cpu.h"
#include <stdio.h>

using namespace cpu;

//return 0 if bit is not 1 in value
#define BITTEST(value,bit) ((value) & (1<<(bit)))
#define BITSET(value,bit) value |= 1 << bit
#define BITCLEAR(value,bit) value &= ~(1 << bit)

#define HALF_CARRY_TEST_ADD(a,b) ((a&0xf) + (b&0xf))&0x10
#define HALF_CARRY_TEST_SUB(a,b) ((a&0xf) - (b&0xf))&0x10

inline void stackPush(CPU& cpu, u16 value)
{
	cpu.SP -= 2; //as we write in reverse, we need to decrease the stack point BEFORE writing
	addresser::writeu16(cpu.addresser, cpu.SP, value);
}

inline void stackPush(CPU& cpu, u8 value)
{
	cpu.SP -= 1;
	addresser::writeu16(cpu.addresser, cpu.SP, value);
}

inline u16 stackPop(CPU& cpu)
{
	cpu.SP += 2;//we should increase AFTER fetching, but to avoid stocking a temp var, we just offset by -2 we reading next
	return addresser::fetchu16(cpu.addresser, cpu.SP - 2);
}

inline void CPAn(CPU& cpu, u8 value)
{
	u8 result = cpu.registers.A - value;

	result == 0x0 ? BITSET(cpu.registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu.registers.F, ZERO_FLAG_BIT);
	BITSET(cpu.registers.F, SUBSTRACT_FLAG_BIT);
	HALF_CARRY_TEST_SUB(cpu.registers.A, value) != 0x0 ? BITSET(cpu.registers.F, HALF_CARRY_FLAG_BIT) : BITCLEAR(cpu.registers.F, HALF_CARRY_FLAG_BIT);

	cpu.registers.A < value ? BITSET(cpu.registers.F, CARRY_FLAG_BIT) : BITCLEAR(cpu.registers.F, CARRY_FLAG_BIT);
}

inline u8 INC8(CPU& cpu, u8 value)
{
	BITSET(cpu.registers.F, SUBSTRACT_FLAG_BIT);
	HALF_CARRY_TEST_ADD(value, (u8)0x1) != 0 ? BITSET(cpu.registers.F, HALF_CARRY_FLAG_BIT) : BITCLEAR(cpu.registers.F, HALF_CARRY_FLAG_BIT);

	value - (u8)0x1 == (u8)0x0 ? BITSET(cpu.registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu.registers.F, ZERO_FLAG_BIT);

	return value + 1;
}

inline u8 DEC8(CPU& cpu, u8 value)
{
	BITSET(cpu.registers.F, SUBSTRACT_FLAG_BIT);
	HALF_CARRY_TEST_SUB(value, (u8)0x1) != 0 ? BITSET(cpu.registers.F, HALF_CARRY_FLAG_BIT) : BITCLEAR(cpu.registers.F, HALF_CARRY_FLAG_BIT);

	value - (u8)0x1 == (u8)0x0 ? BITSET(cpu.registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu.registers.F, ZERO_FLAG_BIT);

	return value - 1;
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
		printf("unknown extended opcode %hhX at address %hX \n", opcode, cpu.PC - 1);
		return -1;
		break;
	}

	return 0;
}

int decode(CPU& cpu, u8 opcode)
{
	//printf("Decoding %hhX\n", opcode);

	//probably can do without, but too lazy to think about it
	u8 buffer;

	switch (opcode)
	{
	case 0x04:// INC B
		{
			cpu.registers.B = INC8(cpu, cpu.registers.B);
		}
		break;
	case 0x05:// DEC B
		{
			cpu.registers.B = DEC8(cpu, cpu.registers.B);
		}
		break;
	case 0x06: // LD B, n
		cpu.registers.B = addresser::fetchu8(cpu.addresser, cpu.PC);
		cpu.PC += 1;
		break;
	case 0x0C:// INC C
		cpu.registers.C = INC8(cpu, cpu.registers.C);
		break;
	case 0x0D:// DEC C
		cpu.registers.C = DEC8(cpu, cpu.registers.C);
		break;
	case 0x0E:
		cpu.registers.C = addresser::fetchu8(cpu.addresser, cpu.PC);
		cpu.PC += 1;
		break;
	case 0x11://LD DE, d16
		cpu.registers.DE = addresser::fetchu16(cpu.addresser, cpu.PC);
		cpu.PC += 2;
		break;
	case 0x13: //INC DE
		cpu.registers.DE += 1;
		break;
	case 0x17://RL A (rotate left, carry goes to bit 0 and bit 7 goes to carry)
		buffer = BITTEST(cpu.registers.A, 7);
		cpu.registers.A = cpu.registers.A << 1;

		BITTEST(cpu.registers.F, CARRY_FLAG_BIT) == 0 ? BITCLEAR(cpu.registers.A, 0) : BITSET(cpu.registers.A, 0);
		buffer == 0 ? BITCLEAR(cpu.registers.F, CARRY_FLAG_BIT) : BITSET(cpu.registers.F, CARRY_FLAG_BIT);
		break;
	case 0x18: //JR r8 (r == relative, it is signed)
		cpu.PC += addresser::fetchs8(cpu.addresser, cpu.PC) + 1;
		break;
	case 0x1A: //LD A, (DE)
		cpu.registers.A = addresser::fetchu8(cpu.addresser, cpu.registers.DE);
		break;
	case 0x20://JR NZ, n (n is signed)
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
	case 0x22: //LD (HL+), A
		addresser::writeu8(cpu.addresser, cpu.registers.HL, cpu.registers.A);
		cpu.registers.HL += 1;
		break;
	case 0x23: // INC HL
		cpu.registers.HL += 1;
		break;
	case 0x28://JR Z, n (n is signed)
		if (BITTEST(cpu.registers.F, ZERO_FLAG_BIT) != 0)
			cpu.PC += addresser::fetchs8(cpu.addresser, cpu.PC) + 1; //+1, because we didn't advance PC after fetching offset.
		else
			cpu.PC += 1;
		break;
	case 0x2E:// LD L, d8
		cpu.registers.L = addresser::fetchu8(cpu.addresser, cpu.PC);
		cpu.PC += 1;
		break;
	case 0x31:
		{//LD SP,d16
			cpu.SP = addresser::fetchu16(cpu.addresser, cpu.PC);
			cpu.PC += 2;
		}
		break;
	case 0x32: //LD (HL-), A
		{
			addresser::writeu8(cpu.addresser, cpu.registers.HL, cpu.registers.A);
			cpu.registers.HL = cpu.registers.HL - 1;
		}
		break;
	case 0x3C: //INC A
		cpu.registers.A = INC8(cpu, cpu.registers.A);
		break;
	case 0x3D: //DEC A
		cpu.registers.A = DEC8(cpu, cpu.registers.A);
		break;
	case 0x3E: //LD A, d8
		cpu.registers.A = addresser::fetchu8(cpu.addresser, cpu.PC);
		cpu.PC += 1;
		break;
	case 0x4F : //LD C, A
		cpu.registers.C = cpu.registers.A;
		break;
	case 0x77: //LD (HL),A
		addresser::writeu8(cpu.addresser, cpu.registers.HL, cpu.registers.A);
		break;
	case 0x7B: //LD A, E
		cpu.registers.A = cpu.registers.E;
		break;
	case 0xAF: //XOR A
		cpu.registers.A ^= cpu.registers.A;
		break;
	case 0xC1: //pop BC
		cpu.registers.BC = stackPop(cpu);
		break;
	case 0xC5://push BC
		stackPush(cpu, cpu.registers.BC);
		break;
	case 0xC9:// RET
		cpu.PC = stackPop(cpu);
		break;
	case 0xCB://extended opcode
		cpu.PC++;
		return extendedDecode(cpu, addresser::fetchu8(cpu.addresser, cpu.PC-1));
		break;
	case 0xCD: //CALL a16
		//we offset the PC by 2, because there is the 16bit address befor ethe next instruction
		stackPush(cpu, (u16)(cpu.PC+2));
		cpu.PC = addresser::fetchu16(cpu.addresser, cpu.PC);
		break;
	case 0xE0:
		addresser::writeu8(cpu.addresser, 0xff00 + addresser::fetchu8(cpu.addresser, cpu.PC), cpu.registers.A);
		cpu.PC += 1;
		break;
	case 0xE2:
		addresser::writeu8(cpu.addresser, 0xff00 + cpu.registers.C, cpu.registers.A);
		break;
	case 0xEA: //LD (a16), A
		addresser::writeu8(cpu.addresser, addresser::fetchu16(cpu.addresser, cpu.PC), cpu.registers.A);
		cpu.PC += 2;
		break;
	case 0xFE:// CP d8
		CPAn(cpu, addresser::fetchu8(cpu.addresser, cpu.PC));
		cpu.PC += 1;
		break;
	default:
		printf("unknown opcode %hhX at address %hX \n", opcode, cpu.PC - 1);
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