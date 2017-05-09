#pragma once

#include "cpu.h"
#include "SDL_stdinc.h"

namespace graphic
{
	void drawTile(CPU& cpu, u16 x, u16 y, u8 tilenum, Uint32* pixels, u16 width);
}