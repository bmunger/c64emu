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
	SystemCpu.Reset();
	SystemMemory.Reset();
	SystemVideo.Reset();
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