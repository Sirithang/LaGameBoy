#pragma once


const int SCREEN_WIDTH = 160;
const int SCREEN_HEIGHT = 144;

const int CLOCK_SPEED_HZ = 4194304;

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed char s8;
typedef signed short s16;

const u8 ZERO_FLAG_BIT = 7;
const u8 SUBSTRACT_FLAG_BIT = 6;
const u8 HALF_CARRY_FLAG_BIT = 5;
const u8 CARRY_FLAG_BIT = 4;

//return 0 if bit is not 1 in value, otherwise return 1
#define BITTEST(value,bit) (((value) & (1<<(bit))) == 0 ? 0 : 1)
#define BITSET(value,bit) value |= 1 << bit
#define BITCLEAR(value,bit) value &= ~(1 << bit)

#define HALF_CARRY_TEST_ADD(a,b) ((a&0xf) + (b&0xf))&0x10
#define HALF_CARRY_TEST_SUB(a,b) ((a&0xf) - (b&0xf))&0x10

#define HALF_CARRY_TEST_ADD_16(a,b) ((a&0xff) + (b&0xff))&0x100

//if pos == 0 extratc the 2 leftmost bit, if 2, the 2 next etc.
#define EXTRACT2BIT(byte, pos) ((byte & (0x3<<pos)) >> (pos))


//==== UGLY huge MACRO for operation on all registry using the opcode destination info (last 3 bit)

//that will take a func(cpu, u8).
#define FUNC_ON_REGISTER(func, dest, cycleReg, cycleMem) \
switch(dest)\
{\
case 0x0:\
func(cpu, cpu->registers.B);\
cycle = cycleReg;\
break;\
case 0x1:\
func(cpu, cpu->registers.C); \
cycle = cycleReg;\
break; \
case 0x2:\
func(cpu, cpu->registers.D);\
cycle = cycleReg;\
break;\
case 0x3:\
func(cpu, cpu->registers.E);\
cycle = cycleReg;\
break;\
case 0x4:\
func(cpu, cpu->registers.H);\
cycle = cycleReg;\
break;\
case 0x5:\
func(cpu, cpu->registers.L);\
cycle = cycleReg;\
break;\
case 0x6:\
func(cpu, motherboard::fetchu8(cpu->mb, cpu->registers.HL));\
cycle = cycleMem;\
break;\
case 0x7:\
func(cpu, cpu->registers.A);\
cycle = cycleReg;\
break;\
}

#define FUNC_ON_REGISTER_PARAM(func, dest, param, cycleReg, cycleMem) \
switch(dest)\
{\
case 0x0:\
func(cpu, cpu->registers.B, param);\
cycle = cycleReg;\
break;\
case 0x1:\
func(cpu, cpu->registers.C, param); \
cycle = cycleReg;\
break; \
case 0x2:\
func(cpu, cpu->registers.D, param);\
cycle = cycleReg;\
break;\
case 0x3:\
func(cpu, cpu->registers.E, param);\
cycle = cycleReg;\
break;\
case 0x4:\
func(cpu, cpu->registers.H, param);\
cycle = cycleReg;\
break;\
case 0x5:\
func(cpu, cpu->registers.L, param);\
cycle = cycleReg;\
break;\
case 0x6:\
func(cpu, motherboard::fetchu8(cpu->mb, cpu->registers.HL), param);\
cycle = cycleMem;\
break;\
case 0x7:\
func(cpu, cpu->registers.A, param);\
cycle = cycleReg;\
break;\
}

//that will take a func(cpu, u8).
#define FUNC_ON_REGISTER_ASSIGN(func, dest, cycleReg, cycleMem) \
switch(dest)\
{\
case 0x0:\
cpu->registers.B = func(cpu, cpu->registers.B);\
cycle = cycleReg;\
break;\
case 0x1:\
cpu->registers.C = func(cpu, cpu->registers.C); \
cycle = cycleReg;\
break; \
case 0x2:\
cpu->registers.D = func(cpu, cpu->registers.D);\
cycle = cycleReg;\
break;\
case 0x3:\
cpu->registers.E = func(cpu, cpu->registers.E);\
cycle = cycleReg;\
break;\
case 0x4:\
cpu->registers.H = func(cpu, cpu->registers.H);\
cycle = cycleReg;\
break;\
case 0x5:\
cpu->registers.L = func(cpu, cpu->registers.L);\
cycle = cycleReg;\
break;\
case 0x6:\
motherboard::writeu8(cpu->mb, cpu->registers.HL, func(cpu, motherboard::fetchu8(cpu->mb, cpu->registers.HL)));\
cycle = cycleMem;\
break;\
case 0x7:\
cpu->registers.A = func(cpu, cpu->registers.A);\
cycle = cycleReg;\
break;\
}

//that will take a func(cpu, u8).
#define FUNC_ON_REGISTER_PARAM_ASSIGN(func, dest, param, cycleReg, cycleMem) \
switch(dest)\
{\
case 0x0:\
cpu->registers.B = func(cpu, cpu->registers.B,param);\
cycle = cycleReg;\
break;\
case 0x1:\
cpu->registers.C = func(cpu, cpu->registers.C,param); \
cycle = cycleReg;\
break; \
case 0x2:\
cpu->registers.D = func(cpu, cpu->registers.D,param);\
cycle = cycleReg;\
break;\
case 0x3:\
cpu->registers.E = func(cpu, cpu->registers.E,param);\
cycle = cycleReg;\
break;\
case 0x4:\
cpu->registers.H = func(cpu, cpu->registers.H,param);\
cycle = cycleReg;\
break;\
case 0x5:\
cpu->registers.L = func(cpu, cpu->registers.L,param);\
cycle = cycleReg;\
break;\
case 0x6:\
motherboard::writeu8(cpu->mb, cpu->registers.HL, func(cpu, motherboard::fetchu8(cpu->mb, cpu->registers.HL),param));\
cycle = cycleMem;\
break;\
case 0x7:\
cpu->registers.A = func(cpu, cpu->registers.A, param);\
cycle = cycleReg;\
break;\
}