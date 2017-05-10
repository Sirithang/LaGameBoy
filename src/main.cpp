#include <SDL.h>
#include <iostream>

#include "cpu.h"
#include "graphic.h"

int main(int argc, char **argv)
{
	SDL_Window* window = NULL;
	SDL_Surface* screenSurface = NULL;
	SDL_Event event;
	int cputick = 1;
	int loop = 1;

	const int tileDebugSize = 16 * 8;
	SDL_Window* debugTileWindow = NULL;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL init failure! SDL_Error: %s\n", SDL_GetError());
	}
	else 
	{
		window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		debugTileWindow = SDL_CreateWindow("Tile Memory", 100, 100, 16 * 8, 16 * 8, SDL_WINDOW_SHOWN);

		if (window == NULL) 
		{
			printf("Window create fail! SDL_Error: %s\n", SDL_GetError());
		}
		else 
		{
			screenSurface = SDL_GetWindowSurface(window);
			SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
			SDL_UpdateWindowSurface(window);

			SDL_Renderer* debugTileRenderer = SDL_CreateRenderer(debugTileWindow, -1, SDL_RENDERER_ACCELERATED);
			SDL_Texture* texture = SDL_CreateTexture(debugTileRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, tileDebugSize, tileDebugSize);
			Uint32* debugTilePixels = new Uint32[tileDebugSize*tileDebugSize];
		
		
			CPU cpu;
			cpu.PC = 0;
			cpu.registers.F = 0;

			cart::load(cpu.addresser.cart, "data/Tetris.gb");
			gpu::init(cpu.addresser.gpu);

			//used to refresh debug display every 70000 cycles;
			int debugRefreshCycle = 0;


			while (loop)
			{
				while (SDL_PollEvent(&event))
				{
					switch (event.type)
					{
					case SDL_QUIT:
						loop = 0;
						break;
					case SDL_KEYDOWN:
						loop = 0;
						break;
					}
				}

				if (cputick > 0)
				{
					int cycle = cpu::tick(cpu);

					if (cycle < 0)
					{
						printf("cpu ticking halted, unknown opcode\n");
						cputick = 0;
					}
					else
					{
						debugRefreshCycle += cycle;

						if (gpu::tick(cpu.addresser.gpu, cycle))
						{
							//refresh display
						}
					}
				}
				else
					debugRefreshCycle += 1;

				if (debugRefreshCycle > 70000)
				{
					debugRefreshCycle -= 70000;
					SDL_RenderClear(debugTileRenderer);
					//tile debug
					for (int y = 0; y < 16; ++y)
					{
						for (int x = 0; x < 16; ++x)
						{
							graphic::drawTile(cpu, x * 8, y * 8, (y * 16) + x, debugTilePixels, tileDebugSize);
						}
					}

					SDL_UpdateTexture(texture, NULL, &debugTilePixels[0], tileDebugSize * 4);
					SDL_RenderCopy(debugTileRenderer, texture, NULL, NULL);
					SDL_RenderPresent(debugTileRenderer);
				}
			}
		}
	}
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}