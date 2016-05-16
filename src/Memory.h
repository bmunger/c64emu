#ifndef _MEMORY_H
#define _MEMORY_H

class Video;
class Memory;
class CIAChip;

// Function pointer type for CIA Chip callbacks.
typedef void (*FnPtrCiaCallback)(CIAChip* chip);

class CIAChip
{
public:
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

protected:

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