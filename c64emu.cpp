#include "c64emu.h"
#include <stdio.h>

int main(int argc, char* argv[])
{
	/* Set up SDL window */
	SDL_Window *main_window;
	SDL_Init(SDL_INIT_VIDEO);
	main_window = SDL_CreateWindow(
		"C64 Emulator",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		800,
		600,
		SDL_WINDOW_OPENGL
	);

	/* Check for errors */
	if (main_window == NULL)
	{
		printf("Error creating window: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Delay(3000);

	/* Begin emulation */

	/* End emulation */
	SDL_DestroyWindow(main_window);
	SDL_Quit();
	return 0;
}
