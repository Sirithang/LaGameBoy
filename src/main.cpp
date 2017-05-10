#include <SDL.h>
#include <iostream>

#include "gameboy.h"
#include "graphic.h"
#include "debugger.h"


int main(int argc, char **argv)
{
	SDL_Window* window = NULL;
	SDL_Surface* screenSurface = NULL;
	SDL_Event event;
	int cputick = 1;
	int loop = 1;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL init failure! SDL_Error: %s\n", SDL_GetError());
	}
	else 
	{
		//debugger init
		DebugDisplay tileDataDisplay;
		debugger::createDebugDisplay(&tileDataDisplay, "Tile Data", 128, 128, 2);

		DebugDisplay bgmapDisplay;
		debugger::createDebugDisplay(&bgmapDisplay, "BG 0", 256, 256, 2);

		window = SDL_CreateWindow("La GameBoy", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH*2, SCREEN_HEIGHT*2, SDL_WINDOW_SHOWN);

		if (window == NULL) 
		{
			printf("Window create fail! SDL_Error: %s\n", SDL_GetError());
		}
		else 
		{
			screenSurface = SDL_GetWindowSurface(window);
			SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
			SDL_UpdateWindowSurface(window);

			//init the display texture
			SDL_Renderer* screenRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
			SDL_Texture* screenTexture = SDL_CreateTexture(screenRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
		

			//Gameboy init
			Motherboard mb;
			mb.cpu.PC = 0;
			mb.cpu.registers.F = 0;

			motherboard::init(&mb);
			cart::load(&mb.cart, "data/Tetris.gb");
			gpu::init(&mb.gpu);

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
					int cycle = cpu::tick(&mb.cpu);

					if (cycle < 0)
					{
						printf("cpu ticking halted, unknown opcode\n");
						cputick = 0;
					}
					else
					{
						debugRefreshCycle += cycle;

						if (gpu::tick(&mb.gpu, cycle))
						{//the gpu got into VBLANK, time to update the screen
							SDL_UpdateTexture(screenTexture, NULL, &mb.gpu.buffer[0], SCREEN_WIDTH * 4);
							SDL_RenderCopy(screenRenderer, screenTexture, NULL, NULL);
							SDL_RenderPresent(screenRenderer);
						}

						motherboard::updateGPURegister(&mb);
					}
				}
				else
					debugRefreshCycle += 10000;

				if (debugRefreshCycle > 70000)
				{
					debugRefreshCycle -= 70000;
					
					debugger::tileDataDebug(&tileDataDisplay, &mb);
					debugger::bgmapDebug(&bgmapDisplay, &mb);

					// Update the screen for debug purpose (what was in the buffer before the CPu halted
					SDL_UpdateTexture(screenTexture, NULL, &mb.gpu.buffer[0], SCREEN_WIDTH * 4);
					SDL_RenderCopy(screenRenderer, screenTexture, NULL, NULL);
					SDL_RenderPresent(screenRenderer);
				}
			}
		}
	}
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}