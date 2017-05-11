#pragma once

const int SCREEN_WIDTH = 160;
const int SCREEN_HEIGHT = 144;

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed char s8;

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

#define HALF_CARRY_TEST_ADD_16(a,b) ((a&0xff) + (b&0xff))&0x100

//if pos == 0 extratc the 2 leftmost bit, if 2, the 2 next etc.
#define EXTRACT2BIT(byte, pos) ((byte & (0x3<<pos)) >> (pos))