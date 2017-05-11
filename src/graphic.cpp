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
				gpu->currentState = 1;
				BITSET(gpu->mb->internalMemory.IORegister[0x0F], 0);
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
			}
		}
	default:
		break;
	}

	return dirty;
}

void gpu::renderLine(GPU* gpu, u8 line)
{
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

	u8 paletteByte = motherboard::fetchu8(gpu->mb, 0xFF47);

	for (u8 x = 0; x < SCREEN_WIDTH; ++x)
	{
		u8 currentXTile = correctedX / 8;

		if (xTileIdx != currentXTile)
		{
			xTileIdx = currentXTile;
			tileX = correctedX - xTileIdx * 8;

			u16 tileIdx = yTileIdx * 32 + xTileIdx;

			tileNum = motherboard::fetchu8(gpu->mb, 0x9800 + tileIdx);
		}

		u8 paletteIdx = graphic::fetchTilePixelPaletteIdx(gpu->mb, tileNum, tileY, tileX);
		u8 shade = EXTRACT2BIT(paletteByte, paletteIdx * 2);

		int offset = (line * SCREEN_WIDTH) + x;
		gpu->buffer[offset] = palette[shade];

		correctedX++;
		tileX++;
	}
}

//------------------------------------------------------

u8 graphic::fetchTilePixelPaletteIdx(Motherboard* mb, u8 tileNum, u8 pixelY, u8 pixelX)
{
	u8 leastByte = motherboard::fetchu8(mb, 0x8000 + tileNum * 16 + pixelY * 2);
	u8 mostByte = motherboard::fetchu8(mb, 0x8000 + tileNum * 16 + pixelX * 2 + 1);

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

			u8 paletteIdx = BITTEST(leastByte, offset) != 0 ? 0x1 : 0x0;
			paletteIdx += BITTEST(mostByte, offset) != 0 ? 0x2 : 0x0;

			u8 shade = EXTRACT2BIT(paletteByte, paletteIdx * 2);
			pixels[(y+py)*width + (x+px)] = palette[shade];
		}
	}
}