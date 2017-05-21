#include "graphic.h"
#include "gameboy.h"

using namespace graphic;

Uint32 palette[4] =
{
	0xFF9CBD0F,
	0xFF8CAD0F,
	0xFF306230,
	0xFF0F380F
};


void gpu::init(GPU* gpu)
{
	gpu->currentTick = 0;
	gpu->currentLine = 0;
	gpu->currentState = 2;
}

bool gpu::tick(GPU* gpu, int tickCount)
{
	bool dirty = false;
	gpu->currentTick += tickCount;

	switch (gpu->currentState)
	{
	case 2: //OAM access (simulated for timing, in emulation, everything is access at once in gpu::renderLine)
		if (gpu->currentTick > 80)
		{
			gpu->currentTick -= 80;
			gpu->currentState = 3;
		}
		break;
	case 3: //Vram & OAM access
		if (gpu->currentTick > 172)
		{
			gpu->currentTick -= 172;
			gpu->currentState = 0;

			if (BITTEST(gpu->mb->internalMemory.IORegister[0x41], 3) != 0x0)
			{//STAT ask for start of HBLank interupt
				BITSET(gpu->mb->internalMemory.IORegister[0x0F], 1);
			}

			gpu::renderLine(gpu, gpu->currentLine);
		}
		break;
	case 0: // HBLANK
		if (gpu->currentTick > 204)
		{
			gpu->currentTick -= 204;
			gpu->currentLine++;

			if (gpu->currentLine == 143)
			{//go into VBLANK
				gpu::renderLine(gpu, gpu->currentLine);

				gpu->currentState = 1;
				BITSET(gpu->mb->internalMemory.IORegister[0x0F], 0);

				if (BITTEST(gpu->mb->internalMemory.IORegister[0x41], 4) != 0x0)
				{//STAT ask for start of VBLANK interupt
					BITSET(gpu->mb->internalMemory.IORegister[0x0F], 1);
				}

				dirty = true;
			}
			else
			{//continue scanning down
				gpu->currentState = 2;
			}
		}
		break;
	case 1: //VBLANK
		if (gpu->currentTick > 456)
		{
			gpu->currentTick -= 456;
			gpu->currentLine++;

			if (gpu->currentLine > 153)
			{//finished VBlank, go back to scanline writing on top
				gpu->currentLine = 0;
				gpu->currentState = 2;
				if (BITTEST(gpu->mb->internalMemory.IORegister[0x41], 5) != 0x0)
				{//STAT ask for start of OAM interupt 
					BITSET(gpu->mb->internalMemory.IORegister[0x0F], 1);
				}
			}
		}
	default:
		break;
	}

	return dirty;
}

void gpu::renderLine(GPU* gpu, u8 line)
{
	u8* OAM = &gpu->mb->internalMemory.OAM[0];

	u8 scrollY = motherboard::fetchu8(gpu->mb, 0xFF42);
	u8 scrollX = motherboard::fetchu8(gpu->mb, 0xFF43);

	u8 correctedY = (scrollY + line);
	u8 correctedX = scrollX;

	u8 yTileIdx = correctedY / 8;
	u8 xTileIdx = correctedX / 8;
	u8 tileIdx = yTileIdx * 32 + xTileIdx;
	u8 tileNum = 0;

	//this is x and y in tile space
	u8 tileY = correctedY - yTileIdx * 8;
	u8 tileX = correctedX - xTileIdx * 8;

	u8 bgPalette = gpu->mb->internalMemory.IORegister[0x47];
	u8 spritePalette = gpu->mb->internalMemory.IORegister[0x48];

	u8 bgNumber = BITTEST(gpu->mb->internalMemory.IORegister[0x40], 3);

	// For each pixel we store which sprite number(0-40) should be displayed here.
	u8 pxielSpritesBuffer[SCREEN_WIDTH];
	SDL_memset(pxielSpritesBuffer, 0xFF, sizeof(u8) * SCREEN_WIDTH);

	if (BITTEST(gpu->mb->internalMemory.IORegister[0x40], 1) != 0)
	{//if bit 1 of 0xff40 isn't set, sprite display is disabled
		//=== Fetch all sprite on that line.

		u8 spriteHeight = BITTEST(gpu->mb->internalMemory.IORegister[0x40], 2) == 0 ? 8 : 16;
		u8 spriteCount = 0;
		for (int s = 0; s < 40 && spriteCount < 10; ++s)
		{
			s16 ySpr = OAM[s * 4];
			ySpr -= 16; // correctedPosition
			s16 yDiff = line - ySpr;

			if (yDiff >= 0 && (yDiff < spriteHeight))
			{
				spriteCount++; //one sprite displayed!
				u8 xSpr = OAM[s * 4 + 1];
				u8 sprNum = OAM[s * 4 + 2];

				for (int xSpriLine = 0; xSpriLine < 8; ++xSpriLine)
				{
					u8 correctedXSpriLine = xSpriLine;

					s16 pixX = xSpr - 8 + xSpriLine;
					if (pixX < 0 || pixX >= SCREEN_WIDTH)
						continue; //pixel outside of screen;

					if (pxielSpritesBuffer[pixX] == 0xFF || OAM[pxielSpritesBuffer[pixX] * 4 + 1] > xSpr)
					{//either no sprite on that line yet OR
						u8 ySpriLine = yDiff;

						u8 attribute = OAM[s * 4 + 3];
						if (BITTEST(attribute, 6) != 0x0) // Y Flip
							ySpriLine = spriteHeight - 1 - ySpriLine;
						if (BITTEST(attribute, 5) != 0x0) // X Flip
							correctedXSpriLine = 7 - xSpriLine;

						u8 correctedSprNum = sprNum;
						if (spriteHeight > 8)
						{//need to correct sprite num if in 8x16
							//correctedSprNum = ySpriLine >= 8 ? sprNum & 0xFE : sprNum | 0x01;
						}

						u8 paletteIdx = graphic::fetchTilePixelPaletteIdx(gpu->mb, correctedSprNum, ySpriLine, correctedXSpriLine, 1);

						if (paletteIdx != 0) //index 0 is transparent for sprite
						{
							u8 shade = EXTRACT2BIT(spritePalette, paletteIdx * 2);
							gpu->buffer[(line * SCREEN_WIDTH) + pixX] = palette[shade];

							//doing that here will probably "fix" some original display bug
							pxielSpritesBuffer[pixX] = s;
						}

					}
				}
			}
		}

		//=== end of sprite writing and storing
	}

	//this will trigger the if condition in the for loop to compute tilenum for 1st pixel
	xTileIdx++;

	for (u8 x = 0; x < SCREEN_WIDTH; ++x)
	{
		u8 currentXTile = correctedX / 8;

		if (xTileIdx != currentXTile)
		{
			xTileIdx = currentXTile;
			tileX = correctedX - xTileIdx * 8;

			u16 tileIdx = yTileIdx * 32 + xTileIdx;

			tileNum = motherboard::fetchu8(gpu->mb, (bgNumber == 0 ? 0x9800 : 0x9C00 )+ tileIdx);
		}

		u8 paletteIdx = graphic::fetchTilePixelPaletteIdx(gpu->mb, tileNum, tileY, tileX, 0);
		u8 shade = EXTRACT2BIT(bgPalette, paletteIdx * 2);

		int offset = (line * SCREEN_WIDTH) + x;

		if (pxielSpritesBuffer[x] != 0xFF)
		{//there is a sprite pixel here, now need to know if set into above or below BG

			if (paletteIdx != 0)
			{//BG palette index 0 is always behind the sprite (i.e. consider it transparent)
				u8 attribute = OAM[pxielSpritesBuffer[x] * 4 + 3];
				if (BITTEST(attribute, 7) != 0x0)
				{//this object is behind the BG so draw the BG
					gpu->buffer[offset] = palette[shade];
				}
			}
		}
		else
		{//no sprite here, just write BG color
			gpu->buffer[offset] = palette[shade];
		}

		correctedX++;
		tileX++;
	}
}

//------------------------------------------------------

u8 graphic::fetchTilePixelPaletteIdx(Motherboard* mb, u8 tileNum, u8 pixelY, u8 pixelX, u8 isOBJ)
{
	u8 leastByte;
	u8 mostByte;

	if (isOBJ == 0 && BITTEST(mb->internalMemory.IORegister[0x40], 4) == 0)
	{//sprite can only 
		leastByte = motherboard::fetchu8(mb, 0x9000 + (s8)tileNum * 16 + pixelY * 2);
		mostByte = motherboard::fetchu8(mb, 0x9000 + (s8)tileNum * 16 + pixelY * 2 + 1);
	}
	else
	{
		leastByte = motherboard::fetchu8(mb, 0x8000 + tileNum * 16 + pixelY * 2);
		mostByte = motherboard::fetchu8(mb, 0x8000 + tileNum * 16 + pixelY * 2 + 1);
	}

	u8 offset = 7 - pixelX;

	u8 paletteIdx = BITTEST(leastByte, offset) != 0 ? 0x1 : 0x0;
	paletteIdx += BITTEST(mostByte, offset) != 0 ? 0x2 : 0x0;

	return paletteIdx;
}

void graphic::drawTile(Motherboard* motherboard, u16 x, u16 y, u8 tilenum, Uint32* pixels, u16 width)
{
	u8 paletteByte = motherboard::fetchu8(motherboard, 0xFF47);


	for (u16 py = 0; py < 8; ++py)
	{
		u8 leastByte = motherboard::fetchu8(motherboard, 0x8000 + tilenum * 16 + py * 2);
		u8 mostByte = motherboard::fetchu8(motherboard, 0x8000 + tilenum * 16 + py * 2 + 1);

		for (u16 px = 0; px < 8; ++px)
		{
			u8 offset = 7 - px;

			//less effective than using the fetched byte above, but allow parity with actual emulation
			//rendering, to show bugs
			u8 paletteIdx = graphic::fetchTilePixelPaletteIdx(motherboard, tilenum, py, px, 1);

			/*u8 paletteIdx = BITTEST(leastByte, offset) != 0 ? 0x1 : 0x0;
			paletteIdx += BITTEST(mostByte, offset) != 0 ? 0x2 : 0x0;*/

			u8 shade = EXTRACT2BIT(paletteByte, paletteIdx * 2);
			pixels[(y+py)*width + (x+px)] = palette[shade];
		}
	}
}