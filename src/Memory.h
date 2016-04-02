#ifndef _MEMORY_H
#define _MEMORY_H

class Video;

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

protected:

	const unsigned char LORAM = 1;
	const unsigned char HIRAM = 2;
	const unsigned char CHAREN = 4;

	unsigned char EffectivePR();

	unsigned char DDR, PR;

	unsigned char * LoadRom(const char * Filename, int Size);

};

#endif