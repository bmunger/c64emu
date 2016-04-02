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
	S = 0xFF;
	X = 0;
	Y = 0;
	PC = (AttachedMemory->Read8(0xFFFC)) | (AttachedMemory->Read8(0xFFFD) << 8);
}

bool Cpu::Step()
{
	unsigned short instructionPC;
	instructionPC = PC;
	unsigned char instruction;
	unsigned char temp, temp2;

	instruction = LoadInstructionByte(); 

	switch (instruction)
	{
	// 0x00 row (Implied/Immediate)
	case 0x20: TRACE_INSTRUCTION("JSR");
		temp = LoadInstructionByte();
		Push(High(PC));
		Push(Low(PC));
		temp2 = LoadInstructionByte();
		SetLow(PC, temp); // Can't modify PC until after we load all the bytes for the instruction.
		SetHigh(PC, temp2);
		break;

	// 0x01 row (Indirect, X)

	case 0x01: TRACE_INSTRUCTION("ORA");
		A = A | Load(LoadInstructionByte() + X);
		SetResultFlags(A);
		break;

	// 0x02 row (Immediate)

	case 0xA2: TRACE_INSTRUCTION("LDX"); // Load X (from immediate)
		X = LoadInstructionByte();
		SetResultFlags(X);
		break;


	// 0x08 row

	case 0x08: TRACE_INSTRUCTION("PHP"); // Push P
		Push(P | BFlag); // B flag is always set when pushing.
		break;

	case 0x28: TRACE_INSTRUCTION("PLP"); // Pull P
		P = Pop() | OneFlag;
		break;

	case 0x48: TRACE_INSTRUCTION("PHA"); // Push A
		Push(A);
		break;

	case 0x68: TRACE_INSTRUCTION("PLA"); // Pull A
		A = Pop();
		SetResultFlags(A);
		break;

	// 0x09 row

	case 0x89: TRACE_INSTRUCTION("NOP"); // Undocumented NOP
		break;




	// 0x18 row
	case 0x18: TRACE_INSTRUCTION("CLC");
		ClearFlag(CFlag);
		break;
	case 0x38: TRACE_INSTRUCTION("SEC");
		SetFlag(CFlag);
		break;

	case 0x58: TRACE_INSTRUCTION("CLI");
		ClearFlag(IFlag);
		break;


	case 0x78: TRACE_INSTRUCTION("SEI");
		SetFlag(IFlag);
		break;

	case 0xB8: TRACE_INSTRUCTION("CLV");
		ClearFlag(VFlag);
		break;


	case 0xD8: TRACE_INSTRUCTION("CLD");
		ClearFlag(DFlag);
		break;
	case 0xF8: TRACE_INSTRUCTION("SED");
		SetFlag(DFlag);
		break;


	// 0x1A row 

	case 0x9A: TRACE_INSTRUCTION("TXS"); // Transfer X to Stack register
		S = X;
		break;

	// 0x1D row (Absolute, X)

	case 0xBD: TRACE_INSTRUCTION("LDA"); // Load a from [x]
		A = Load(X);
		SetResultFlags(A);
		break;


	default:
		TRACE_INSTRUCTION("Unrecognized Instruction");
		return false; // CPU does not recognize this instruction.

	}




	return true;
}

unsigned char Cpu::Load(unsigned short Address)
{
	return AttachedMemory->Read8(Address);
}

// Set N/Z based on result.
void Cpu::SetResultFlags(unsigned char Result)
{
	// Set Zero and Negative flags.
	P = P & ~(ZFlag | NFlag);
	P |= (Result & 0x80); // N flag
	if (Result == 0)
	{
		P |= ZFlag;
	}
}

void Cpu::SetFlag(unsigned char Flag)
{
	P = P | Flag;
}
void Cpu::ClearFlag(unsigned char Flag)
{
	P = P & (~Flag);
}

unsigned char Cpu::Low(unsigned short Reg)
{
	return Reg & 0xFF;
}
unsigned char Cpu::High(unsigned short Reg)
{
	return (Reg >> 8) & 0xFF;
}
void Cpu::SetLow(unsigned short & Reg, unsigned char NewLow)
{
	Reg = (Reg & 0xFF00) | NewLow;
}
void Cpu::SetHigh(unsigned short & Reg, unsigned char NewHigh)
{
	Reg = (Reg & 0x00FF) | ((unsigned short)NewHigh << 8);
}

void Cpu::Push(unsigned char Value)
{
	AttachedMemory->Write8(0x100 | S, Value);
	S--;
}
unsigned char Cpu::Pop()
{
	S++;
	return AttachedMemory->Read8(0x100 | S);
}

unsigned char Cpu::LoadInstructionByte()
{
	unsigned char byte = AttachedMemory->Read8(PC);
	PC++;
	return byte;
}