#include "Video.h"
#include "Memory.h"
#include "Cpu.h"

#define GENERATE_COLOR(r,g,b) (((r)<<16) | ((g)<<8) | (b) | 0xFF000000)

Video::Video()
{
	ScreenWidth = 411;
	ScreenHeight = 234;

	ScreenData = new unsigned int[ScreenWidth * ScreenHeight];

	// Colors randomly entered, probably way off.
	Colors[0] = GENERATE_COLOR(0, 0, 0); // Black
	Colors[1] = GENERATE_COLOR(255, 255, 255); // White
	Colors[2] = GENERATE_COLOR(255, 0, 0); // Red
	Colors[3] = GENERATE_COLOR(0, 255, 255); // Cyan
	Colors[4] = GENERATE_COLOR(255, 192, 192); // Pink
	Colors[5] = GENERATE_COLOR(0, 255, 0); // Green
	Colors[6] = GENERATE_COLOR(0, 0, 255); // Blue
	Colors[7] = GENERATE_COLOR(255, 255, 0); // Yellow
	Colors[8] = GENERATE_COLOR(255, 128, 0); // Orange
	Colors[9] = GENERATE_COLOR(128, 128, 0); // Brown
	Colors[10] = GENERATE_COLOR(255, 128, 128); // Light Red
	Colors[11] = GENERATE_COLOR(64, 64, 64); // Dark Grey
	Colors[12] = GENERATE_COLOR(128, 128, 128); // Medium Gray
	Colors[13] = GENERATE_COLOR(128, 255, 128); // Light Green
	Colors[14] = GENERATE_COLOR(128, 128, 255); // Light Blue
	Colors[15] = GENERATE_COLOR(192, 192, 192); // Light Gray
}


Video::~Video()
{
}

void Video::Reset()
{
	CursorX = 0;
	CursorY = 0;
	PrevCycle = 0;

	// Set these based on the video mode.
	StartX = 24; EndX = 343;
	StartY = 51-28; EndY = 250-28;
}

// Video emulation reference http://www.cebix.net/VIC-Article.txt
// Just picking one of the variants to emulate and will make it more generic later.
// Details being used here:
// 64 cycles per line, 411 visible pixels per line
// 234 visible lines, 262 lines total, 262*64 = 16768  cyeles per frame
// With a system clock of 1022.7khz is 60.991 Hz
// Every clock cycle, the code will advance the raster cursor and fill in the pixels that were rendered in that time
// This approximates what the real hardware would do. It's not quite as precise for a few reasons.

void Video::VideoStep()
{
	int cycles = AttachedCpu->Cycle - PrevCycle;
	PrevCycle += cycles;

	int pixels = cycles * 8;
	while (pixels-- > 0)
	{
		int paletteColor = 0;
		if (CursorX < StartX || CursorX > EndX || CursorY < StartY || CursorY > EndY)
		{
			// Currently in the border region, or offscreen.
			paletteColor = 6;
		}
		else
		{
			// Currently in the display region.
			int renderX = CursorX - StartX;
			int renderY = CursorY - StartY;

			// Todo: Different rendering mechanisms for each type of mode.


		}

		if (CursorX < ScreenWidth && CursorY < ScreenHeight)
		{
			SetPixel(CursorX, CursorY, paletteColor);
		}

		// Advance to the next pixel location.
		CursorX++;
		if (CursorX == 512)
		{
			CursorX = 0;
			CursorY++;
			if (CursorY == 256)
			{
				CursorY = 0;
			}
		}
	}
}

void Video::SetPixel(int X, int Y, int PaletteIndex)
{
	if (X < 0 || Y < 0 || X >= ScreenWidth || Y >= ScreenHeight)
		return;

	ScreenData[X + Y*ScreenWidth] = Colors[PaletteIndex];
}

void Video::SetupRendering(SDL_Window* EmuWindow)
{
	AttachedWindow = EmuWindow;
	Renderer = SDL_CreateRenderer(EmuWindow, -1, SDL_RENDERER_ACCELERATED);
	Screen = SDL_CreateTexture(Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, ScreenWidth, ScreenHeight);
	

}
void Video::TeardownRendering()
{
	SDL_DestroyTexture(Screen);
	SDL_DestroyRenderer(Renderer);
}
void Video::UpdateVideo()
{
	// copy shadow pixel data into the texture
	SDL_UpdateTexture(Screen, NULL, ScreenData, ScreenWidth*sizeof(unsigned long));

	// Draw texture to screen
	SDL_RenderClear(Renderer);
	SDL_RenderCopy(Renderer, Screen, NULL, NULL);
	SDL_RenderPresent(Renderer);
}