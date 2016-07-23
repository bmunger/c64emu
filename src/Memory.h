#ifndef _MEMORY_H
#define _MEMORY_H

#include "EmulationEvent.h"

class Emulation;
class Video;
class Memory;
class Keyboard;
class CIAChip;

// Function pointer type for CIA Chip callbacks.
typedef void (*FnPtrCiaCallback)(CIAChip* chip);

class CIAChip
{
public:
	CIAChip(int CpuInterruptSourceIndex);

	void Setup(Memory* useMemory, FnPtrCiaCallback readFn, FnPtrCiaCallback writeFn);


	void Reset();
	void Write8(int Address, unsigned char Data8);
	unsigned char Read8(int Address);

	Memory* AttachedMemory;

	// Callback called pre-read, use this to set the current state of IO lines.
	FnPtrCiaCallback CbRead;

	// Callback called post-write, use this to update any hardware that needs to react to a change in IO lines.
	FnPtrCiaCallback CbWrite;

	unsigned char PRA, PRB, DDRA, DDRB;

	unsigned char PrevPRA, PrevPRB; // stores the previous state of PRA/PRB to identify changes in IO output.

	int TAValue, TALatch, TBValue, TBLatch;
	int CRA, CRB;
	int IntFlags, IntMask;
	int MaskedFlags;

	long long LastEventA, LastEventB;

protected:

	void SetIntFlags(int flags);
	void ClearIntFlags(int flags);
	void UpdateInterruptStatus();

	void UpdateTimerA();
	void UpdateTimerB();
	void AdvanceTimerA(int counts);
	void AdvanceTimerB(int counts);
	void StartTimerA();
	void StartTimerB();
	void TimerAUnderflow(int count);

	int InterruptSourceIndex;

	EventRequest evtTimerA, evtTimerB;

	static void CallbackTimerA(EventRequest* Request);
	static void CallbackTimerB(EventRequest* Request);

	// CIA specific values
	const int CR_START = 1;
	// 1 = Ouput to PB6
	const int CR_PBON = 2;
	// 1=Toggle 0=Pulse
	const int CR_OUTMODE = 4;
	// 1 = one-shot, 0 = continuous.
	const int CR_RUNMODE = 8;
	// If bit is set while written, force timer latch into timer value
	const int CR_LOAD = 16;
	// 1 = Count CNT transitions, 0=count on CLK edges
	const int CRA_INMODE = 32;
	// 1 = Serial port output, 0=serial port input (external clock)
	const int CRA_SPMODE = 64;
	// 1 = 50Hz input for RTC, 0 = 60Hz input for RTC.
	const int CRA_TODIN = 128;

	const int CRB_INMODE_MASK = 0x60;

	const int CRB_INMODE_CLK = 0;
	const int CRB_INMODE_CNT = 0x20;
	const int CRB_INMODE_TA = 0x40;
	const int CRB_INMODE_TACNT = 0x60;

};


class Memory
{
public:
	Memory();
	~Memory();

	void Reset();

	void Write8(int Address, unsigned char Data8);
	unsigned char Read8(int Address);

	Video * AttachedVideo;
	Cpu * AttachedCpu;
	Keyboard* AttachedKeyboard;
	Emulation* AttachedEmulation;

	unsigned char * RAM;
	unsigned char * Kernal;
	unsigned char * Basic;
	unsigned char * Char;

	CIAChip CIA1, CIA2;

protected:

	const unsigned char LORAM = 1;
	const unsigned char HIRAM = 2;
	const unsigned char CHAREN = 4;

	unsigned char EffectivePR();

	unsigned char DDR, PR;

	unsigned char * LoadRom(const char * Filename, int Size);


	static void Cia1Read(CIAChip* chip);
	static void Cia1Write(CIAChip* chip);
	static void Cia2Read(CIAChip* chip);
	static void Cia2Write(CIAChip* chip);

};





#endif