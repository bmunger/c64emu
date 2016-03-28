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


protected:
	unsigned char EffectivePR();

	unsigned char DDR, PR;
	unsigned char * RAM;
	unsigned char * Kernal;
	unsigned char * Basic;

};

#endif