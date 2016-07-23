#ifndef _EMULATIONEVENT_H
#define _EMULATIONEVENT_H

// This is a shared structure that would otherwise cause some circular dependencies between header files, so storing it in a separate header file.

class EventRequest;

typedef void(*EventCallback)(EventRequest* Request);

class EventRequest
{
public:
	EventRequest(EventCallback CallbackFunction, void* CallbackContext)
	{
		Callback = CallbackFunction;
		Context = CallbackContext;
	}

	long long CallbackTime; // Cycle to callback on.
	bool Queued;
	EventCallback Callback;
	void* Context;
};

#endif;