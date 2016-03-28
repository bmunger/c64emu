#include "Emulation.h"


Emulation::Emulation() : SystemCpu(), SystemMemory(), SystemVideo()
{
	// connect
	SystemVideo.AttachedCpu = &SystemCpu;
	SystemVideo.AttachedMemory = &SystemMemory;
	SystemMemory.AttachedVideo = &SystemVideo;
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

void Emulation::Run()
{
	while (true)
	{
		bool success = SystemCpu.Step();
		if (!success)
		{
			return;
		}

		SystemVideo.VideoStep();
	}

}