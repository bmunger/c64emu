#include "c64emu.h"
#include <stdio.h>
#include "Emulation.h"

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

	/* Begin emulation */
	Emulation emu;

	emu.SetupRendering(main_window);

	while (1) {
		SDL_Event e;
		if (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				break;
			}
		}

		emu.RunCycles(20000);

		emu.UpdateVideo();

		SDL_Delay(1);
	}

	/* End emulation */
	emu.TeardownRendering();
	SDL_DestroyWindow(main_window);
	SDL_Quit();
	return 0;
}
