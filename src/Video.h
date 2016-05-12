#ifndef _VIDEO_H
#define _VIDEO_H

#include "c64emu.h"
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

	void Write8(int Address, unsigned char Data8);
	unsigned char Read8(int Address);

protected:
	SDL_Window* AttachedWindow;
	SDL_Texture* Screen;
	SDL_Renderer* Renderer;

	void SetPixel(int X, int Y, int PaletteIndex);
	void UpdateMode();
	unsigned char ReadVicMemory(int Address);


	int ScreenWidth, ScreenHeight;
	int CursorX, CursorY;
	long long PrevCycle;

	int StartX, EndX, StartY, EndY;
	unsigned int Colors[16];
	unsigned int * ScreenData;

	unsigned char Registers[64];
	unsigned char ColorRam[1024];

};

#endif
