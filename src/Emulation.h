#ifndef _EMULATION_H
#define _EMULATION_H

#include "SDL.h"
#include "Video.h"
#include "Memory.h"
#include "Cpu.h"

class Emulation
{
public:
	Emulation();
	~Emulation();

	void Reset();
	void RunCycles(int CycleCount);

	void SetupRendering(SDL_Window* Target);
	void TeardownRendering();
	void UpdateVideo();

	Video SystemVideo;
	Memory SystemMemory;
	Cpu SystemCpu;
};

#endif