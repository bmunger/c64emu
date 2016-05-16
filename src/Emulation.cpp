#include "Emulation.h"


Emulation::Emulation() : SystemCpu(), SystemMemory(), SystemVideo()
{
	// connect
	SystemVideo.AttachedCpu = &SystemCpu;
	SystemVideo.AttachedMemory = &SystemMemory;
	SystemMemory.AttachedVideo = &SystemVideo;
	SystemMemory.AttachedCpu = &SystemCpu;
	SystemCpu.AttachedMemory = &SystemMemory;

	Reset();
}


Emulation::~Emulation()
{
}


void Emulation::Reset()
{
	SystemMemory.Reset();
	SystemVideo.Reset();
	// Reset CPU last, it loads from memory.
	SystemCpu.Reset();
}

void Emulation::RunCycles(int CycleCount)
{
	long long targetCycle = SystemCpu.Cycle + CycleCount;
	while (SystemCpu.Cycle < targetCycle)
	{
		bool success = SystemCpu.Step();
		if (!success)
		{
			return;
		}

		SystemVideo.VideoStep();
	}

}

void Emulation::SetupRendering(SDL_Window* Target)
{
	SystemVideo.SetupRendering(Target);
}
void Emulation::TeardownRendering()
{
	SystemVideo.TeardownRendering();
}
void Emulation::UpdateVideo()
{
	SystemVideo.UpdateVideo();
}