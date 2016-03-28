#include "Cpu.h"
#include "Memory.h"
#include <stdio.h>

// Intended to be used in Cpu::Step
#define TRACE_INSTRUCTION(message) printf("PC=%04X: %02X A=%02X P=%02X S=%02X X=%02X Y=%02X %s\n", instructionPC, instruction, A, P, S, X, Y, (message))


Cpu::Cpu()
{
}


Cpu::~Cpu()
{
}

void Cpu::Reset()
{
	A = 0;
	P = 0;
	S = 0;
	X = 0;
	Y = 0;
	PC = (AttachedMemory->Read8(0xFFFC) << 8) | AttachedMemory->Read8(0xFFFD);
}

bool Cpu::Step()
{
	unsigned short instructionPC;
	instructionPC = PC;
	unsigned char instruction;

	instruction = AttachedMemory->Read8(PC);
	PC++;





	TRACE_INSTRUCTION("Unrecognized Instruction");
	return false; // CPU does not recognize this instruction.
}
