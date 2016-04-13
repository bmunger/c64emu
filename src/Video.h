#ifndef _VIDEO_H
#define _VIDEO_H

#include "SDL.h"
class Memory;
class Cpu;

class Video
{
public:
	Video();
	~Video();

	void Reset();
	void VideoStep();

	void SetupRendering(SDL_Window* EmuWindow);
	void TeardownRendering();
	void UpdateVideo();

	Memory * AttachedMemory;
	Cpu * AttachedCpu;

protected:
	SDL_Window* AttachedWindow;
	SDL_Texture* Screen;
	SDL_Renderer* Renderer;

	void SetPixel(int X, int Y, int PaletteIndex);

	int ScreenWidth, ScreenHeight;
	int CursorX, CursorY;
	long long PrevCycle;

	int StartX, EndX, StartY, EndY;
	unsigned int Colors[16];
	unsigned int * ScreenData;
};

#endif