#include <SDL.h>
#include <iostream>

#include "cpu.h"

int main(int argc, char **argv)
{
	SDL_Window* window = NULL;
	SDL_Surface* screenSurface = NULL;
	SDL_Event event;
	int cputick = 1;
	int loop = 1;

	CPU cpu;
	cpu.PC = 0;
	cpu.registers.F = 0;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL init failure! SDL_Error: %s\n", SDL_GetError());
	}
	else 
	{
		window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (window == NULL) 
		{
			printf("Window create fail! SDL_Error: %s\n", SDL_GetError());
		}
		else 
		{
			screenSurface = SDL_GetWindowSurface(window);
			SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
			SDL_UpdateWindowSurface(window);
			

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

					if (cputick)
					{
						cputick = cpu::tick(cpu) < 0 ? 0 : 1;

						if (cputick == 0)
							printf("cpu ticking halted, unknown opcode\n");
					}
				}
			}
		}
	}
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}