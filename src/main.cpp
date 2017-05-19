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

	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0) {
		printf("SDL init failure! SDL_Error: %s\n", SDL_GetError());
	}
	else 
	{
		//debugger init
		DebugDisplay tileDataDisplay;
		debugger::createDebugDisplay(&tileDataDisplay, "Tile Data", 128, 128, 2);

		DebugDisplay bgmapDisplay;
		debugger::createDebugDisplay(&bgmapDisplay, "BG 0", 256, 256, 2);
		u8 currentBg = 0;

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
			cart::load(&mb.cart, "data/Zelda.gb");
			gpu::init(&mb.gpu);

			//used to refresh debug display every 70000 cycles;
			int debugRefreshCycle = 0;
			debugger::s_callHistory.currentCall = 0;

			Uint32 frameStart = SDL_GetTicks();
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
						if (event.key.keysym.scancode == SDL_SCANCODE_B)
							currentBg = 1 - currentBg;
					case SDL_KEYUP:
						switch (event.key.keysym.scancode)
						{
						case SDL_SCANCODE_X:
							event.key.state == SDL_PRESSED ? BITCLEAR(mb.inputState, 0) : BITSET(mb.inputState, 0);
							break;
						case SDL_SCANCODE_C:
							event.key.state == SDL_PRESSED ? BITCLEAR(mb.inputState, 1) : BITSET(mb.inputState, 1);
							break;
						case SDL_SCANCODE_BACKSPACE:
							event.key.state == SDL_PRESSED ? BITCLEAR(mb.inputState, 2) : BITSET(mb.inputState, 2);
							break;
						case SDL_SCANCODE_RETURN:
							event.key.state == SDL_PRESSED ? BITCLEAR(mb.inputState, 3) : BITSET(mb.inputState, 3);
							break;
						case SDL_SCANCODE_RIGHT:
							event.key.state == SDL_PRESSED ? BITCLEAR(mb.inputState, 4) : BITSET(mb.inputState, 4);
							break;
						case SDL_SCANCODE_LEFT:
							event.key.state == SDL_PRESSED ? BITCLEAR(mb.inputState, 5) : BITSET(mb.inputState, 5);
							break;
						case SDL_SCANCODE_UP:
							event.key.state == SDL_PRESSED ? BITCLEAR(mb.inputState, 6) : BITSET(mb.inputState, 6);
							break;
						case SDL_SCANCODE_DOWN:
							event.key.state == SDL_PRESSED ? BITCLEAR(mb.inputState, 7) : BITSET(mb.inputState, 7);
							break;
						default:break;
						}
						
						break;
					}
				}

				if (cputick > 0)
				{
					int cycle = cpu::tick(&mb.cpu);

					debugger::s_callHistory.currentCall++;

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

							Uint32 frameEnd = SDL_GetTicks();
							if (frameEnd - frameStart < 16)
							{
								SDL_Delay(16 - (frameEnd - frameStart));
							}

							frameStart = SDL_GetTicks();
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
					debugger::bgmapDebug(&bgmapDisplay, &mb, currentBg);
					//debugger::bgmapDebug(&bgmap1Display, &mb, 1);

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