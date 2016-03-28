#ifndef _CPU_H
#define _CPU_H

class Memory;

class Cpu
{
public:
	Cpu();
	~Cpu();

	void Reset();
	bool Step();

	Memory * AttachedMemory;

protected:

	unsigned short PC; // Program counter
	unsigned char S; // Stack pointer
	unsigned char P; // Processor status
	unsigned char A; // Accumulator
	unsigned char X; // Index register X
	unsigned char Y; // Index register Y

};

#endif