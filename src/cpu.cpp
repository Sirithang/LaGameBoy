#include "opcodes.inl"
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

int extendedDecode(CPU* cpu, u8 opcode)
{
	//probably can do without, but too lazy to think about it
	u8 buffer;
	int cycle = -1;

	u8 dest = opcode & 0x7;
	u8 cleanOpcode = opcode & (~dest);

	switch (cleanOpcode)
	{
	case 0x10://RL dest
		FUNC_ON_REGISTER_ASSIGN(RL, dest, 8, 16);
		break;
	case 0x20:
		FUNC_ON_REGISTER_ASSIGN(SLA, dest, 8, 16);
		break;
	case 0x30://SWAP dest
		FUNC_ON_REGISTER_ASSIGN(SWAP, dest, 8, 16);
		break;
	case 0x38://SRL dest
		FUNC_ON_REGISTER_ASSIGN(SRL, dest, 8, 16);
		break;
	case 0x40: //BIT 0, dest
		FUNC_ON_REGISTER_PARAM(BIT, dest, 0, 8, 16);
		break;
	case 0x48: //BIT 0, dest
		FUNC_ON_REGISTER_PARAM(BIT, dest, 1, 8, 16);
		break;
	case 0x50 : //BIT 2, dest
		FUNC_ON_REGISTER_PARAM(BIT, dest, 2, 8, 16);
		break;
	case 0x58: //BIT 3, dest
		FUNC_ON_REGISTER_PARAM(BIT, dest, 3, 8, 16);
		break;
	case 0x60://BIT 4, dest
		FUNC_ON_REGISTER_PARAM(BIT, dest, 4, 8, 16);
		break;
	case 0x68://BIT 5, dest
		FUNC_ON_REGISTER_PARAM(BIT, dest, 5, 8, 16);
		break;
	case 0x70://BIT 6, dest
		FUNC_ON_REGISTER_PARAM(BIT, dest, 6, 8, 16);
		break;
	case 0x78://BIT 7, dest
		FUNC_ON_REGISTER_PARAM(BIT, dest, 7, 8, 16);
		break;
	case 0x80://RES 0, dest
		FUNC_ON_REGISTER_PARAM_ASSIGN(RES, dest, 0, 8, 16);
		break;
	case 0x98:
		FUNC_ON_REGISTER_PARAM_ASSIGN(RES, dest, 3, 8, 16);
	case 0xA8://RES 5,dest
		FUNC_ON_REGISTER_PARAM_ASSIGN(RES, dest, 5, 8, 16);
		break;
	case 0xF0://SET 6, dest
		FUNC_ON_REGISTER_PARAM_ASSIGN(SET, dest, 6, 8, 16);
		break;
	case 0xF8://SET 7, dest
		FUNC_ON_REGISTER_PARAM_ASSIGN(SET, dest, 7, 8, 16);
		break;
	default:
		printf("unknown EXTENDED opcode %hhX at address %hX \n", opcode, cpu->PC - 1);
		break;
	}

	return cycle;
}

int decode(CPU* cpu, u8 opcode)
{
	//probably can do without, but too lazy to think about it
	u8 buffer;

	int cycle = -1;
	u8 dest = opcode & 0x7;

	switch (opcode)
	{
	case 0x00:
		cycle = 4;
		break;
	case 0x01://LD BC,d16
		cpu->registers.BC = motherboard::fetchu16(cpu->mb, cpu->PC);
		cpu->PC += 2;
		cycle = 12;
		break;
	case 0x02://LD (BC), A
		motherboard::writeu8(cpu->mb, cpu->registers.BC, cpu->registers.A);
		cycle = 8;
		break;
	case 0x03:// INC BC
		cpu->registers.BC += 1;
		cycle = 8;
		break;
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
	case 0x07://RLCA
		cpu->registers.A = RLC(cpu, cpu->registers.A, 0);
		cycle = 4;
		break;
	case 0x09: //ADD HL, BC
		cpu->registers.HL = ADD(cpu, cpu->registers.HL, cpu->registers.BC);
		cycle = 8;
		break;
	case 0x0A://LD A, (BC)
		cpu->registers.A = motherboard::fetchu8(cpu->mb, cpu->registers.BC);
		cycle = 8;
		break;
	case 0x0B: //DEC BC
		cpu->registers.BC -= 1;
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
	case 0x0F:// RRCA
		cpu->registers.A = RRC(cpu, cpu->registers.A, 0);
		cycle = 4;
		break;
	case 0x11://LD DE, d16
		cpu->registers.DE = motherboard::fetchu16(cpu->mb, cpu->PC);
		cpu->PC += 2;
		cycle = 12;
		break;
	case 0x12:// LD(DE), A
		motherboard::writeu8(cpu->mb, cpu->registers.DE, cpu->registers.A);
		cycle = 8;
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

		cpu->registers.F = 0;
		buffer == 0 ? BITCLEAR(cpu->registers.F, CARRY_FLAG_BIT) : BITSET(cpu->registers.F, CARRY_FLAG_BIT);
		cycle = 4;
		break;
	case 0x18: //JR r8 (r == relative, it is signed)
		cpu->PC += motherboard::fetchs8(cpu->mb, cpu->PC) + 1;
		cycle = 12;
		break;
	case 0x19: //ADD HL, DE
		cpu->registers.HL = ADD(cpu, cpu->registers.HL, cpu->registers.DE);
		cycle = 8;
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
	case 0x26: //LD H,d8
		cpu->registers.H = motherboard::fetchu8(cpu->mb, cpu->PC);
		cpu->PC += 1;
		cycle = 8;
		break;
	case 0x27://DAA
		DAA(cpu);
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
	case 0x29: //ADD HL, HL
		cpu->registers.HL = ADD(cpu, cpu->registers.HL, cpu->registers.HL);
		cycle = 8;
		break;
	case 0x2A: //LD A,(HL+)
		cpu->registers.A = motherboard::fetchu8(cpu->mb, cpu->registers.HL);
		cpu->registers.HL += 1;
		cycle = 8;
		break;
	case 0x2C: //INC L
		cpu->registers.L = INC8(cpu, cpu->registers.L);
		cycle = 4;
		break;
	case 0x2D: //DEC L
		cpu->registers.L = DEC8(cpu, cpu->registers.L);
		cycle = 4;
		break;
	case 0x2E:// LD L, d8
		cpu->registers.L = motherboard::fetchu8(cpu->mb, cpu->PC);
		cpu->PC += 1;
		cycle = 8;
		break;
	case 0x2F: // CPL
		cpu->registers.A = ~cpu->registers.A;
		BITSET(cpu->registers.F, SUBSTRACT_FLAG_BIT);
		BITSET(cpu->registers.F, HALF_CARRY_FLAG_BIT);
		cycle = 4;
		break;
	case 0x30://JR NC,r8
		if (BITTEST(cpu->registers.F, CARRY_FLAG_BIT) == 0)
		{
			//+1 because we didn't advance after the fetch
			cpu->PC += motherboard::fetchs8(cpu->mb, cpu->PC) + 1;
			cycle = 12;
		}
		else
		{
			//jump the relative jump count
			cpu->PC += 1;
			cycle = 8;
		}
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
	case 0x34: //INC (HL)
		motherboard::writeu8(cpu->mb, cpu->registers.HL, INC8(cpu, motherboard::fetchu8(cpu->mb, cpu->registers.HL)));
		cycle = 12;
		break;
	case 0x35: //DEC (HL)
		motherboard::writeu8(cpu->mb, cpu->registers.HL, DEC8(cpu, motherboard::fetchu8(cpu->mb, cpu->registers.HL)));
		cycle = 12;
		break;
	case 0x36://LD (HL),d8
		motherboard::writeu8(cpu->mb, cpu->registers.HL, motherboard::fetchu8(cpu->mb, cpu->PC));
		cpu->PC += 1;
		cycle = 12;
		break;
	case 0x39: //ADD HL, SP
		cpu->registers.HL = ADD(cpu, cpu->registers.HL, cpu->SP);
		cycle = 8;
		break;
	case 0x3A: //LD A,(HL-)
		cpu->registers.A = motherboard::fetchu8(cpu->mb, cpu->registers.HL);
		cpu->registers.HL -= 1;
		cycle = 8;
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
	case 0x40: //LD B,B
		cpu->registers.B = cpu->registers.B;
		cycle = 4;
		break;
	case 0x41: //LD B,C
		cpu->registers.B = cpu->registers.C;
		cycle = 4;
		break;
	case 0x42: //LD B,D
		cpu->registers.B = cpu->registers.D;
		cycle = 4;
		break;
	case 0x43: //LD B,E
		cpu->registers.B = cpu->registers.E;
		cycle = 4;
		break;
	case 0x44: //LD B,H
		cpu->registers.B = cpu->registers.H;
		cycle = 4;
		break;
	case 0x45: //LD B,L
		cpu->registers.B = cpu->registers.L;
		cycle = 4;
		break;
	case 0x46: //LD B,(HL)
		cpu->registers.B = motherboard::fetchu8(cpu->mb, cpu->registers.HL);
		cycle = 8;
		break;
	case 0x47: //LD B,A
		cpu->registers.B = cpu->registers.A;
		cycle = 4;
		break;
	case 0x48: //LD C,B
		cpu->registers.C = cpu->registers.B;
		cycle = 4;
		break;
	case 0x49: //LD C, C
		cpu->registers.C = cpu->registers.C;
		cycle = 4;
		break;
	case 0x4A: //LD C, D
		cpu->registers.C = cpu->registers.D;
		cycle = 4;
		break;
	case 0x4B: //LD C, E
		cpu->registers.C = cpu->registers.E;
		cycle = 4;
		break;
	case 0x4C: //LD C,H
		cpu->registers.C = cpu->registers.H;
		cycle = 4;
		break;
	case 0x4D: //LD C,L
		cpu->registers.C = cpu->registers.L;
		cycle = 4;
		break;
	case 0x4E: //LD C,(HL)
		cpu->registers.C = motherboard::fetchu8(cpu->mb, cpu->registers.HL);
		cycle = 8;
		break;
	case 0x4F : //LD C, A
		cpu->registers.C = cpu->registers.A;
		cycle = 4;
		break;
	case 0x50: //LD D, B
		cpu->registers.D = cpu->registers.B;
		cycle = 4;
		break;
	case 0x51: //LD D, C
		cpu->registers.D = cpu->registers.C;
		cycle = 4;
		break;
	case 0x52: //LD D, D
		cpu->registers.D = cpu->registers.D;
		cycle = 4;
		break;
	case 0x53: //LD D, E
		cpu->registers.D = cpu->registers.E;
		cycle = 4;
		break;
	case 0x54: //LD D, H
		cpu->registers.D = cpu->registers.H;
		cycle = 4;
		break;
	case 0x55: //LD D, L
		cpu->registers.D = cpu->registers.L;
		cycle = 4;
		break;
	case 0x56: //LD D, (HL)
		cpu->registers.D = motherboard::fetchu8(cpu->mb, cpu->registers.HL);
		cycle = 8;
		break;
	case 0x57://LD D,A
		cpu->registers.D = cpu->registers.A;
		cycle = 4;
		break;
	case 0x58://LD E,B
		cpu->registers.E = cpu->registers.B;
		cycle = 4;
		break;
	case 0x59://LD E,C
		cpu->registers.E = cpu->registers.C;
		cycle = 4;
		break;
	case 0x5A://LD E,D
		cpu->registers.E = cpu->registers.D;
		cycle = 4;
		break;
	case 0x5B://LD E,E
		cpu->registers.E = cpu->registers.E;
		cycle = 4;
		break;
	case 0x5C://LD E,H
		cpu->registers.E = cpu->registers.H;
		cycle = 4;
		break;
	case 0x5D://LD E,L
		cpu->registers.E = cpu->registers.L;
		cycle = 4;
		break;
	case 0x5E://LD E,(HL)
		cpu->registers.E = motherboard::fetchu8(cpu->mb, cpu->registers.HL);
		cycle = 8;
		break;
	case 0x5F://LD E,A
		cpu->registers.E = cpu->registers.A;
		cycle = 4;
		break;
	case 0x60://LD H, B
		cpu->registers.H = cpu->registers.B;
		cycle = 4;
		break;
	case 0x61://LD H, C
		cpu->registers.H = cpu->registers.C;
		cycle = 4;
		break;
	case 0x62://LD H, D
		cpu->registers.H = cpu->registers.D;
		cycle = 4;
		break;
	case 0x63://LD H, E
		cpu->registers.H = cpu->registers.E;
		cycle = 4;
		break;
	case 0x64://LD H, H
		cpu->registers.H = cpu->registers.H;
		cycle = 4;
		break;
	case 0x65://LD H, L
		cpu->registers.H = cpu->registers.L;
		cycle = 4;
		break;
	case 0x66://LD H, (HL)
		cpu->registers.H = motherboard::fetchu8(cpu->mb, cpu->registers.HL);
		cycle = 8;
		break;
	case 0x67://LD H, A
		cpu->registers.H = cpu->registers.A;
		cycle = 4;
		break;
	case 0x68://LD L, B
		cpu->registers.L = cpu->registers.B;
		cycle = 4;
		break;
	case 0x69://LD L, C
		cpu->registers.L = cpu->registers.C;
		cycle = 4;
		break;
	case 0x6A://LD L, D
		cpu->registers.L = cpu->registers.D;
		cycle = 4;
		break;
	case 0x6B://LD L, E
		cpu->registers.L = cpu->registers.E;
		cycle = 4;
		break;
	case 0x6C://LD L, H
		cpu->registers.L = cpu->registers.H;
		cycle = 4;
		break;
	case 0x6D://LD L, L
		cpu->registers.L = cpu->registers.L;
		cycle = 4;
		break;
	case 0x6E://LD L, (HL)
		cpu->registers.L = motherboard::fetchu8(cpu->mb, cpu->registers.HL);
		cycle = 8;
		break;
	case 0x6F://LD L, A
		cpu->registers.L = cpu->registers.A;
		cycle = 4;
		break;
	case 0x70: //LD (HL),B
		motherboard::writeu8(cpu->mb, cpu->registers.HL, cpu->registers.B);
		cycle = 8;
		break;
	case 0x71: //LD (HL),C
		motherboard::writeu8(cpu->mb, cpu->registers.HL, cpu->registers.C);
		cycle = 8;
		break;
	case 0x72: //LD (HL),D
		motherboard::writeu8(cpu->mb, cpu->registers.HL, cpu->registers.D);
		cycle = 8;
		break;
	case 0x73: //LD (HL),E
		motherboard::writeu8(cpu->mb, cpu->registers.HL, cpu->registers.E);
		cycle = 8;
		break;
	case 0x74: //LD (HL),H
		motherboard::writeu8(cpu->mb, cpu->registers.HL, cpu->registers.H);
		cycle = 8;
		break;
	case 0x75: //LD (HL),L
		motherboard::writeu8(cpu->mb, cpu->registers.HL, cpu->registers.L);
		cycle = 8;
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
	case 0x7E: //LD A, (HL)
		cpu->registers.A = motherboard::fetchu8(cpu->mb, cpu->registers.HL);
		cycle = 8;
		break;
	case 0x7F: //LD A,A
		cpu->registers.A = cpu->registers.A;
		cycle = 4;
		break;
	case 0x80: //ADD A,B
		cpu->registers.A = ADD(cpu, cpu->registers.A, cpu->registers.B);
		cycle = 4;
		break;
	case 0x81: //ADD A,C
		cpu->registers.A = ADD(cpu, cpu->registers.A, cpu->registers.C);
		cycle = 4;
		break;
	case 0x82: //ADD A,D
		cpu->registers.A = ADD(cpu, cpu->registers.A, cpu->registers.D);
		cycle = 4;
		break;
	case 0x83: //ADD A,E
		cpu->registers.A = ADD(cpu, cpu->registers.A, cpu->registers.E);
		cycle = 4;
		break;
	case 0x84: //ADD A,H
		cpu->registers.A = ADD(cpu, cpu->registers.A, cpu->registers.H);
		cycle = 4;
		break;
	case 0x85: //ADD A,L
		cpu->registers.A = ADD(cpu, cpu->registers.A, cpu->registers.L);
		cycle = 4;
		break;
	case 0x86: //ADD A,(HL)
		cpu->registers.A = ADD(cpu, cpu->registers.A, motherboard::fetchu8(cpu->mb, cpu->registers.HL));
		cycle = 8;
		break;
	case 0x87: //ADD A,A
		cpu->registers.A = ADD(cpu, cpu->registers.A, cpu->registers.A);
		cycle = 4;
		break;
	case 0x88: //ADC A,B
		cpu->registers.A = ADD(cpu, cpu->registers.A, cpu->registers.B + BITTEST(cpu->registers.F, CARRY_FLAG_BIT));
		cycle = 4;
		break;
	case 0x89: //ADC A,C
		cpu->registers.A = ADD(cpu, cpu->registers.A, cpu->registers.C + BITTEST(cpu->registers.F, CARRY_FLAG_BIT));
		cycle = 4;
		break;
	case 0x8A: //ADC A,D
		cpu->registers.A = ADD(cpu, cpu->registers.A, cpu->registers.D + BITTEST(cpu->registers.F, CARRY_FLAG_BIT));
		cycle = 4;
		break;
	case 0x8B: //ADC A,E
		cpu->registers.A = ADD(cpu, cpu->registers.A, cpu->registers.E + BITTEST(cpu->registers.F, CARRY_FLAG_BIT));
		cycle = 4;
		break;
	case 0x8C: //ADC A,H
		cpu->registers.A = ADD(cpu, cpu->registers.A, cpu->registers.H + BITTEST(cpu->registers.F, CARRY_FLAG_BIT));
		cycle = 4;
		break;
	case 0x8D: //ADC A,L
		cpu->registers.A = ADD(cpu, cpu->registers.A, cpu->registers.L + BITTEST(cpu->registers.F, CARRY_FLAG_BIT));
		cycle = 4;
		break;
	case 0x8E: //ADC A,(HL)
		cpu->registers.A = ADD(cpu, cpu->registers.A, motherboard::fetchu8(cpu->mb, cpu->registers.HL) + BITTEST(cpu->registers.F, CARRY_FLAG_BIT));
		cycle = 8;
		break;
	case 0x8F: //ADC A,A
		cpu->registers.A = ADD(cpu, cpu->registers.A, cpu->registers.A + BITTEST(cpu->registers.F, CARRY_FLAG_BIT));
		cycle = 4;
		break;
	case 0x90: // SUB B
		cpu->registers.A = SUB(cpu, cpu->registers.A, cpu->registers.B);
		cycle = 4;
		break;
	case 0xA0: // AND B
		cpu->registers.A &= cpu->registers.B;
		cpu->registers.F = 0;
		cpu->registers.A == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		BITSET(cpu->registers.F, HALF_CARRY_FLAG_BIT);
		cycle = 4;
		break;
	case 0xA1: // AND C
		cpu->registers.A &= cpu->registers.C;
		cpu->registers.F = 0;
		cpu->registers.A == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		BITSET(cpu->registers.F, HALF_CARRY_FLAG_BIT);
		cycle = 4;
		break;
	case 0xA2: //AND D
		cpu->registers.A &= cpu->registers.D;
		cpu->registers.F = 0;
		cpu->registers.A == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		BITSET(cpu->registers.F, HALF_CARRY_FLAG_BIT);
		cycle = 4;
		break;
	case 0xA3: //AND E
		cpu->registers.A &= cpu->registers.E;
		cpu->registers.F = 0;
		cpu->registers.A == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		BITSET(cpu->registers.F, HALF_CARRY_FLAG_BIT);
		cycle = 4;
		break;
	case 0xA4: //AND H
		cpu->registers.A &= cpu->registers.H;
		cpu->registers.F = 0;
		cpu->registers.A == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		BITSET(cpu->registers.F, HALF_CARRY_FLAG_BIT);
		cycle = 4;
		break;
	case 0xA5: //AND L
		cpu->registers.A &= cpu->registers.L;
		cpu->registers.F = 0;
		cpu->registers.A == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		BITSET(cpu->registers.F, HALF_CARRY_FLAG_BIT);
		cycle = 4;
		break;
	case 0xA6: //AND (HL)
		cpu->registers.A &= motherboard::fetchu8(cpu->mb, cpu->registers.HL);
		cpu->registers.F = 0;
		cpu->registers.A == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		BITSET(cpu->registers.F, HALF_CARRY_FLAG_BIT);
		cycle = 8;
		break;
	case 0xA7: //AND A
		cpu->registers.A &= cpu->registers.A;
		cpu->registers.F = 0;
		cpu->registers.A == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		BITSET(cpu->registers.F, HALF_CARRY_FLAG_BIT);
		cycle = 4;
		break;
	case 0xA8: //XOR B
		cpu->registers.A ^= cpu->registers.B;
		cpu->registers.F = 0;
		cpu->registers.A == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		cycle = 4;
		break;
	case 0xA9: //XOR C
		cpu->registers.A ^= cpu->registers.C;
		cpu->registers.F = 0;
		cpu->registers.A == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		cycle = 4;
		break;
	case 0xAA: //XOR D
		cpu->registers.A ^= cpu->registers.D;
		cpu->registers.F = 0;
		cpu->registers.A == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		cycle = 4;
		break;
	case 0xAB: //XOR E
		cpu->registers.A ^= cpu->registers.E;
		cpu->registers.F = 0;
		cpu->registers.A == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		cycle = 4;
		break;
	case 0xAC: //XOR H
		cpu->registers.A ^= cpu->registers.H;
		cpu->registers.F = 0;
		cpu->registers.A == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		cycle = 4;
		break;
	case 0xAD: //XOR L
		cpu->registers.A ^= cpu->registers.L;
		cpu->registers.F = 0;
		cpu->registers.A == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		cycle = 4;
		break;
	case 0xAE: //XOR (HL)
		cpu->registers.A ^= motherboard::fetchu8(cpu->mb, cpu->registers.HL);
		cpu->registers.F = 0;
		cpu->registers.A == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		cycle = 8;
		break;
	case 0xAF: //XOR A
		cpu->registers.A ^= cpu->registers.A;
		cpu->registers.F = 0;
		cpu->registers.A == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		cycle = 4;
		break;
	case 0xB0: // OR B
		cpu->registers.A |= cpu->registers.B;
		cpu->registers.F = 0;
		cpu->registers.A == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		cycle = 4;
		break;
	case 0xB1: // OR C
		cpu->registers.A |= cpu->registers.C;
		cpu->registers.F = 0;
		cpu->registers.A == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		cycle = 4;
		break;
	case 0xB2: // OR D
		cpu->registers.A |= cpu->registers.D;
		cpu->registers.F = 0;
		cpu->registers.A == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		cycle = 4;
		break;
	case 0xB3: // OR E
		cpu->registers.A |= cpu->registers.E;
		cpu->registers.F = 0;
		cpu->registers.A == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		cycle = 4;
		break;
	case 0xB4: // OR H
		cpu->registers.A |= cpu->registers.H;
		cpu->registers.F = 0;
		cpu->registers.A == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		cycle = 4;
		break;
	case 0xB5: // OR L
		cpu->registers.A |= cpu->registers.L;
		cpu->registers.F = 0;
		cpu->registers.A == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		cycle = 4;
		break;
	case 0xB6: // OR (HL)
		cpu->registers.A |= motherboard::fetchu8(cpu->mb, cpu->registers.HL);
		cpu->registers.F = 0;
		cpu->registers.A == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		cycle = 8;
		break;
	case 0xB7: // OR A
		cpu->registers.A |= cpu->registers.A;
		cpu->registers.F = 0;
		cpu->registers.A == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		cycle = 4;
		break;
	case 0xB8: //CP B
		CPAn(cpu, motherboard::fetchu8(cpu->mb, cpu->registers.B));
		cycle = 4;
		break;
	case 0xB9: //CP C
		CPAn(cpu, motherboard::fetchu8(cpu->mb, cpu->registers.C));
		cycle = 4;
		break;
	case 0xBA: //CP D
		CPAn(cpu, motherboard::fetchu8(cpu->mb, cpu->registers.D));
		cycle = 4;
		break;
	case 0xBB: //CP E
		CPAn(cpu, motherboard::fetchu8(cpu->mb, cpu->registers.E));
		cycle = 4;
		break;
	case 0xBC: //CP H
		CPAn(cpu, motherboard::fetchu8(cpu->mb, cpu->registers.H));
		cycle = 4;
		break;
	case 0xBD: //CP L
		CPAn(cpu, motherboard::fetchu8(cpu->mb, cpu->registers.L));
		cycle = 4;
		break;
	case 0xBE: //CP (HL)
		CPAn(cpu, motherboard::fetchu8(cpu->mb, cpu->registers.HL));
		cycle = 8;
		break;
	case 0xBF: //CP L
		CPAn(cpu, motherboard::fetchu8(cpu->mb, cpu->registers.A));
		cycle = 4;
		break;
	case 0xC0: //RET NZ
		if (BITTEST(cpu->registers.F, ZERO_FLAG_BIT) == 0)
		{//return
			cpu->PC = stackPop(cpu);
			cycle = 20;
		}
		else
		{//pass
			cycle = 8;
		}
		break;
	case 0xC1: //pop BC
		cpu->registers.BC = stackPop(cpu);
		cycle = 12;
		break;
	case 0xC2: //JP NZ,a16
		if (BITTEST(cpu->registers.F, ZERO_FLAG_BIT) == 0)
		{//jump
			cpu->PC = motherboard::fetchu16(cpu->mb, cpu->PC);
			cycle = 16;
		}
		else
		{//continue
			//jump the address
			cpu->PC += 2;
			cycle = 12;
		}
		break;
	case 0xC3:// JP a16
		cpu->PC = motherboard::fetchu16(cpu->mb, cpu->PC);
		cycle = 16;
		break;
	case 0xC5://push BC
		stackPush(cpu, cpu->registers.BC);
		cycle = 16;
		break;
	case 0xC6://ADD A,d8
		cpu->registers.A = ADD(cpu, cpu->registers.A, motherboard::fetchu8(cpu->mb, cpu->PC));
		cpu->PC += 1;
		cycle = 8;
		break;
	case 0xC8: //RET Z
		if (BITTEST(cpu->registers.F, ZERO_FLAG_BIT) != 0)
		{//jump
			cpu->PC = stackPop(cpu);
			cycle = 20;
		}
		else
		{
			cycle = 8;
		}
		break;
	case 0xC9:// RET
		cpu->PC = stackPop(cpu);
		cycle = 16;
		break;
	case 0xCA: //JP Z,a16
		if (BITTEST(cpu->registers.F, ZERO_FLAG_BIT) != 0)
		{
			cpu->PC = motherboard::fetchu16(cpu->mb, cpu->PC);
			cycle = 16;
		}
		else
		{
			cpu->PC += 2;
			cycle = 12;
		}
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
	case 0xD0: //RET NC
		if (BITTEST(cpu->registers.F, CARRY_FLAG_BIT) == 0)
		{//jump
			cpu->PC = stackPop(cpu);
			cycle = 20;
		}
		else
		{//continue
			cycle = 8;
		}
		break;
	case 0xD1://POP DE
		cpu->registers.DE = stackPop(cpu);
		cycle = 12;
		break;
	case 0xD5: //PUSH DE
		stackPush(cpu, cpu->registers.DE);
		cycle = 16;
		break;
	case 0xD6: //SUB d8
		cpu->registers.A = SUB(cpu, cpu->registers.A, motherboard::fetchu8(cpu->mb, cpu->PC));
		cpu->PC += 1;
		cycle = 8;
		break;
	case 0xD9: //RETI
		cpu->PC = stackPop(cpu);
		cpu->interruptEnabled = 1;
		cycle = 16;
		break;
	case 0xDF://RST 18H
		stackPush(cpu, cpu->PC);
		cpu->PC = 0x18;
		cycle = 16;
		break;
	case 0xE0: //LDH (a8), A
		motherboard::writeu8(cpu->mb, 0xff00 + motherboard::fetchu8(cpu->mb, cpu->PC), cpu->registers.A);
		cpu->PC += 1;
		cycle = 12;
		break;
	case 0xE1: // POP HL
		cpu->registers.HL = stackPop(cpu);
		cycle = 12;
		break;
	case 0xE2:// LD (C), A
		motherboard::writeu8(cpu->mb, 0xff00 + cpu->registers.C, cpu->registers.A);
		cycle = 12;
		break;
	case 0xE5: //PUSH HL
		stackPush(cpu, cpu->registers.HL);
		cycle = 16;
		break;
	case 0xE6: //AND d8
		cpu->registers.A = cpu->registers.A & motherboard::fetchu8(cpu->mb, cpu->PC);
		cpu->PC += 1;
		cpu->registers.F = 0;
		cpu->registers.A == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		BITSET(cpu->registers.F, HALF_CARRY_FLAG_BIT);
		cycle = 8;
		break;
	case 0xE9: //JP (HL)
		cpu->PC = cpu->registers.HL;
		cycle = 4;
		break;
	case 0xEA: //LD (a16), A
		motherboard::writeu8(cpu->mb, motherboard::fetchu16(cpu->mb, cpu->PC), cpu->registers.A);
		cpu->PC += 2;
		cycle = 16;
		break;
	case 0xEE: //XOR d8
		cpu->registers.A = cpu->registers.A ^ motherboard::fetchu8(cpu->mb, cpu->PC);
		cpu->PC += 1;
		cpu->registers.F = 0;
		cpu->registers.A == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		cycle = 8;
		break;
	case 0xEF://RST 28H
		stackPush(cpu, cpu->PC);
		cpu->PC = 0x28;
		cycle = 16;
		break;
	case 0xF0: // LDH A,(a8)
		cpu->registers.A = motherboard::fetchu8(cpu->mb, 0xFF00 + motherboard::fetchu8(cpu->mb, cpu->PC));
		cpu->PC += 1;
		cycle = 12;
		break;
	case 0xF1://POP AF
		cpu->registers.AF = stackPop(cpu);
		cycle = 12;
		break;
	case 0xF3: // DI
		cpu->interruptEnabled = 0;
		cycle = 4;
		break;
	case 0xF5: //PUSH AF
		stackPush(cpu, cpu->registers.AF);
		cycle = 16;
		break;
	case 0xF6: //OR d8
		cpu->registers.A = cpu->registers.A | motherboard::fetchu8(cpu->mb, cpu->PC);
		cpu->PC += 1;
		cpu->registers.F = 0;
		cpu->registers.A == 0 ? BITSET(cpu->registers.F, ZERO_FLAG_BIT) : BITCLEAR(cpu->registers.F, ZERO_FLAG_BIT);
		cycle = 8;
		break;
	case 0xFA: //LD A,(a16)
		cpu->registers.A = motherboard::fetchu8(cpu->mb, motherboard::fetchu16(cpu->mb, cpu->PC));
		cpu->PC += 2;
		cycle = 16;
		break;
	case 0xFB://EI
		cpu->interruptEnabled = 1;
		cycle = 4;
		break;
	case 0xFE:// CP d8
		CPAn(cpu, motherboard::fetchu8(cpu->mb, cpu->PC));
		cpu->PC += 1;
		cycle = 8;
		break;
	case 0xFF://RST 38H
		stackPush(cpu, cpu->PC);
		cpu->PC = 0x38;
		cycle = 16;
		break;
	default:
		printf("unknown opcode %hhX at address %hX \n", opcode, cpu->PC - 1);
		break;
	}

	return cycle;
}

u8 checkInterrupts(CPU* cpu)
{
	if (cpu->interruptEnabled == 0)
		return 0;

	if (BITTEST(cpu->mb->internalMemory.InterruptRegister, 0) != 0 &&
		BITTEST(cpu->mb->internalMemory.IORegister[0x0F], 0) != 0)
	{//V blank interupt enabled & asked
		BITCLEAR(cpu->mb->internalMemory.IORegister[0x0F], 0);
		cpu->interruptEnabled = 0;
		stackPush(cpu, cpu->PC);
		cpu->PC = 0x40;
		return 5;
	}

	if (BITTEST(cpu->mb->internalMemory.InterruptRegister, 1) != 0 &&
		BITTEST(cpu->mb->internalMemory.IORegister[0x0F], 1) != 0)
	{//LCD STAT Interupt enabled & asked
		BITCLEAR(cpu->mb->internalMemory.IORegister[0x0F], 1);
		cpu->interruptEnabled = 0;
		stackPush(cpu, cpu->PC);
		cpu->PC = 0x48;
		return 5;
	}

	return 0;
}

int cpu::tick(CPU* cpu)
{
	//if a DMA request is pending, execute it
	//(on hardware would be parallel to CPu execution but for emulation
	//intantaneous on a CPU cycle should be enough. Hopefully.
	if (cpu->mb->DMARequested)
	{
		cpu->mb->DMARequested = 0;
		SDL_memcpy(
			motherboard::fetchu8p(cpu->mb, 0xFE00),
			motherboard::fetchu8p(cpu->mb, cpu->mb->internalMemory.IORegister[0x46] * 0x100),
			(0x9F + 1) * sizeof(u8));
	}

	u8 interruptCycle = checkInterrupts(cpu);

	if (interruptCycle != 0)
		return interruptCycle;

	u8 opcode = motherboard::fetchu8(cpu->mb, cpu->PC);
	cpu->PC += 1;

	return decode(cpu, opcode);
}