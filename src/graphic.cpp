#include "graphic.h"

using namespace graphic;

void graphic::drawTile(CPU& cpu, u16 x, u16 y, u8 tilenum, Uint32* pixels, u16 width)
{
	for (u16 py = 0; py < 8; ++py)
	{
		u8 leastByte = addresser::fetchu8(cpu.addresser, 0x8000 + tilenum * 16 + py * 2);
		u8 mostByte = addresser::fetchu8(cpu.addresser, 0x8000 + tilenum * 16 + py * 2 + 1);

		for (u16 px = 0; px < 8; ++px)
		{
			u8 offset = 7 - px;

			u8 shade = BITTEST(leastByte, offset) != 0 ? 0x1 : 0x0;
			shade += BITTEST(mostByte, offset) != 0 ? 0x2 : 0x0;

			//pixels[((y+py) * destination->w) + (x+px)] = (int)floor(0xFFFFFFFF * (shade / 3.0f));
			pixels[(y+py)*width + (x+px)] = shade * 0x3FFFFFFF;
		}
	}
}