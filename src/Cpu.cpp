#include "Cpu.h"
#include "Memory.h"
#include <stdio.h>


#define TRACE_CPU_INSTRUCTIONS 1


#define TRACE_INSTRUCTION_COMMON(message) printf("PC=%04X: %02X A=%02X P=%02X S=%02X X=%02X Y=%02X : %s\n", instructionPC, instruction, A, P, S, X, Y, (message))


#if TRACE_CPU_INSTRUCTIONS
// Intended to be used in Cpu::Step
#define TRACE_INSTRUCTION(message) TRACE_INSTRUCTION_COMMON(message)
#define TRACE_SPRINTF sprintf

#else
#define TRACE_INSTRUCTION(message)
#define TRACE_SPRINTF ignore_args

static void ignore_args(...)
{

}

#endif

#define TRACE_UNDEFINED(message) TRACE_INSTRUCTION_COMMON(message)

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
	PC = Load16(0xFFFC);
}

bool Cpu::Step()
{
	unsigned short instructionPC;
	instructionPC = PC;
	unsigned char instruction;
	unsigned char temp, temp2;
	unsigned short stemp;
	char disasm[16];

	instruction = LoadInstructionByte(); 

	switch (instruction)
	{
	// 0x00 row (Implied/Immediate)
	case 0x20: //TRACE_INSTRUCTION("JSR");
		temp = LoadInstructionByte();
		Push(High(PC));
		Push(Low(PC));
		temp2 = LoadInstructionByte();
		SetLow(PC, temp); // Can't modify PC until after we load all the bytes for the instruction.
		SetHigh(PC, temp2);
		TRACE_SPRINTF(disasm, "JSR $%02X%02X", temp2, temp);
		TRACE_INSTRUCTION(disasm);
		break;


	case 0x60: TRACE_INSTRUCTION("RTS"); // Return from subroutine
		temp = Pop();
		temp2 = Pop();
		SetLow(PC, temp);
		SetHigh(PC, temp2);
		PC++;
		break;

	case 0xD0: //TRACE_INSTRUCTION("BNE");
		if ((P&ZFlag) == 0)
		{
			temp = LoadInstructionByte();
			PC += (char)temp; // Signed offset
		}
		TRACE_SPRINTF(disasm, "BNE $%04X,X", PC);
		TRACE_INSTRUCTION(disasm);
		break;

	// 0x01 row (Indirect, X)

	case 0x01: TRACE_INSTRUCTION("ORA");
		A = A | Load(LoadInstructionByte() + X);
		SetResultFlags(A);
		break;

	// 0x02 row (Immediate)

	case 0xA2: //TRACE_INSTRUCTION("LDX"); // Load X (from immediate)
		X = LoadInstructionByte();
		SetResultFlags(X);
		TRACE_SPRINTF(disasm, "LDX #$%02X", X);
		TRACE_INSTRUCTION(disasm);
		break;

	// 0x05 row
	case 0x85: // Store A
		temp = LoadInstructionByte();
		AttachedMemory->Write8(temp, A);
		TRACE_SPRINTF(disasm, "STA $%02X", temp);
		TRACE_INSTRUCTION(disasm);
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

	case 0xA9: //Load Accumulator
		A = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "LDA #$%02X", A);
		TRACE_INSTRUCTION(disasm);
		break;

	// 0x0A row

	case 0xCA: TRACE_INSTRUCTION("DEX"); // Decrement X
		X--;
		SetResultFlags(X);
		break;

	// 0x0D row

	case 0x8D: // Store A
		stemp = LoadInstructionShort();
		AttachedMemory->Write8(stemp, A);
		TRACE_SPRINTF(disasm, "STA $%04X", stemp);
		TRACE_INSTRUCTION(disasm);
		break;

	// 0x0E row
	case 0x8E: // Store X
		stemp = LoadInstructionShort();
		AttachedMemory->Write8(stemp, X);
		TRACE_SPRINTF(disasm, "STX $%04X", stemp);
		TRACE_INSTRUCTION(disasm);
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

	case 0xBD: //TRACE_INSTRUCTION("LDA"); // Load a from [Immediate16+X]
		stemp = LoadInstructionShort();
		//A = Load(LoadInstructionShort() + X);
		A = Load(stemp) + X;
		SetResultFlags(A);
		TRACE_SPRINTF(disasm, "LDA $%04X,X", stemp);
		TRACE_INSTRUCTION(disasm);
		break;

	case 0xDD: //TRACE_INSTRUCTION("CMP"); // Compare A with [Immediate16+X]
		stemp = LoadInstructionShort();
		//Sub(A, Load(LoadInstructionShort() + X), 0);
		Sub(A, Load(stemp + X), 0);
		TRACE_SPRINTF(disasm, "CMP $%04X,X", stemp);
		TRACE_INSTRUCTION(disasm);
		break;

	default:
		TRACE_UNDEFINED("Unrecognized Instruction");
		return false; // CPU does not recognize this instruction.

	}




	return true;
}

unsigned char Cpu::Load(unsigned short Address)
{
	return AttachedMemory->Read8(Address);
}

unsigned short Cpu::Load16(unsigned short Address)
{
	return (AttachedMemory->Read8(Address)) | (AttachedMemory->Read8(Address + 1) << 8);
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

// Add 2 8-bit numbers together and set flags. Only set Overflow flag if adding with carry.
unsigned char Cpu::Add(unsigned char Add1, unsigned char Add2, int Carry)
{
	// Preserve information so we can identify the carry into and carry out of the top bit.
	int c = 0;
	if (Carry != 0)
		c = (P&CFlag) == 0 ? 0 : Carry; // Todo: this probably needs to change slightly for borrow.

	int temp1 = (Add1 & 0x7F) + (Add2 & 0x7F) + c; // Carry in is temp1 & 0x80
	int temp2 = Add1 + Add2 + c; // Carry out is temp2 & 0x100
	unsigned char Result = temp2 & 0xFF;

	P = P & ~(ZFlag | NFlag | CFlag);
	P |= (Result & 0x80); // N flag
	if (Result == 0)
	{
		P |= ZFlag;
	}
	if (temp2 & 0x100) // Carry out
	{
		P |= CFlag;
	}

	if (Carry != 0)
	{
		// Also set overflow flag
		P &= ~VFlag;
		temp1 = (temp1 & 0x80) >> 7;
		temp2 = (temp2 & 0x100) >> 8;
		if (temp1 != temp2)
		{
			P |= VFlag;
		}
	}
	return Result;
}
// Subtract 2 8-bit numbers and set flags
unsigned char Cpu::Sub(unsigned char Sub1, unsigned char Sub2, int Carry)
{
	return Add(Sub1, -Sub2, -Carry);
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
	unsigned char byte = Load(PC);
	PC++;
	return byte;
}

unsigned short Cpu::LoadInstructionShort()
{
	unsigned short data = Load16(PC);
	PC += 2;
	return data;
}
