#include "Emulation.h"


Emulation::Emulation() : SystemCpu(), SystemMemory(), SystemVideo(), SystemKeyboard()
{
	// connect
	SystemVideo.AttachedCpu = &SystemCpu;
	SystemVideo.AttachedMemory = &SystemMemory;
	SystemMemory.AttachedVideo = &SystemVideo;
	SystemMemory.AttachedCpu = &SystemCpu;
	SystemMemory.AttachedKeyboard = &SystemKeyboard;
	SystemMemory.AttachedEmulation = this;
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

	// Reset event system.
	QueuedRequests.clear();
	NextCallbackTime = 0;
}

void Emulation::RunCycles(int CycleCount)
{
	long long targetCycle = SystemCpu.Cycle + CycleCount;
	while (SystemCpu.Cycle < targetCycle)
	{
		if (SystemCpu.Cycle > NextCallbackTime)
		{
			HandleCallbacks();
		}

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


// Request a callback at a certain cycle time
void Emulation::QueueEvent(long long CallbackTime, EventRequest* Request)
{
	if (Request->Queued)
	{
		// Remove the request first if it's already in the queue.
		QueuedRequests.remove(Request);
	}

	// Insert the request in sorted order into the list of requests - so the earliest events will fire first.
	std::list<EventRequest*>::iterator i;
	for (i = QueuedRequests.begin(); i != QueuedRequests.end(); i++)
	{
		if (Request->CallbackTime < (*i)->CallbackTime)
		{
			// New request should go before this entry in the list.
			break;
		}
	}
	QueuedRequests.insert(i, Request);

	Request->Queued = true;
	Request->CallbackTime = CallbackTime;

	SetNextCallbackTime();
}

// Cancel a request for a previously requested callback.
void Emulation::CancelEvent(EventRequest* Request)
{
	size_t before, after;
	before = QueuedRequests.size();
	QueuedRequests.remove(Request);
	after = QueuedRequests.size();
	if (before != after)
	{
		// Element was removed.
		if (Request->Queued != true)
		{
			SDL_TriggerBreakpoint(); // Bad state. The object was in the list but it wasn't marked as queued.
		}
	}
	Request->Queued = false;
	SetNextCallbackTime();
}

void Emulation::HandleCallbacks()
{
	long long curCycle = SystemCpu.Cycle;
	while (QueuedRequests.size() > 0)
	{
		EventRequest* cur = QueuedRequests.front();
		if (curCycle < cur->CallbackTime)
		{
			// It's not time for the first entry in the list yet.
			break;
		}
		else
		{
			// We did pass the callback time for this entry. Remove it from the list and trigger the callback.
			QueuedRequests.pop_front();
			cur->Queued = false;

			cur->Callback(cur);
		}
	}
	SetNextCallbackTime();
}

void Emulation::SetNextCallbackTime()
{
	long long nextCycle = SystemCpu.Cycle + 1000000;
	if (QueuedRequests.size() > 0)
	{
		long long reqCycle = QueuedRequests.front()->CallbackTime;
		if (reqCycle < nextCycle)
		{
			nextCycle = reqCycle;
		}
	}
	NextCallbackTime = nextCycle;
}