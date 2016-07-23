#ifndef _EMULATION_H
#define _EMULATION_H

#include "c64emu.h"
#include "EmulationEvent.h"
#include "Video.h"
#include "Memory.h"
#include "Cpu.h"
#include "Keyboard.h"

#include <list>

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
	Keyboard SystemKeyboard;

	// Request a callback at a certain cycle time
	void QueueEvent(long long CallbackTime, EventRequest* Request);
	// Cancel a request for a previously requested callback.
	void CancelEvent(EventRequest* Request);

protected:
	long long NextCallbackTime;
	std::list<EventRequest*> QueuedRequests;
	void HandleCallbacks();
	void SetNextCallbackTime();
};




#endif
