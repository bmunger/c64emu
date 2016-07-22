#include "Cpu.h"
#include "Memory.h"
#include <stdio.h>

// Print out every CPU instruction (debug purposes)
#define TRACE_CPU_INSTRUCTIONS 1

// Combined with above, actually only print out the last instructions before an undefined instruction.
#define TRACE_BUFFER_ON_UNDEFINED 1


#define TRACE_INSTRUCTION_COMMON(message) printf("PC=%04X: %02X A=%02X P=%02X S=%02X X=%02X Y=%02X : %s (%lld)\n", instructionPC, instruction, A, P, S, X, Y, (message), Cycle)

#if TRACE_CPU_INSTRUCTIONS

#if TRACE_BUFFER_ON_UNDEFINED

const int TraceBufferCount = 128;
const int TraceBufferLineSize = 512;
char TraceSaveBuffer[TraceBufferCount][TraceBufferLineSize] = {};
int TraceBufferIndex = 0;

#define TRACE_INSTRUCTION_SAVE(message) sprintf(TraceSaveBuffer[TraceBufferIndex], "PC=%04X: %02X A=%02X P=%02X S=%02X X=%02X Y=%02X : %s\n", instructionPC, instruction, A, P, S, X, Y, (message)); TraceBufferIndex = (TraceBufferIndex+1)%TraceBufferCount

#define TRACE_INSTRUCTION(message) TRACE_INSTRUCTION_SAVE(message)
#define TRACE_SPRINTF sprintf

void DumpTraceBuffers()
{
	for (int i = 0; i < TraceBufferCount; i++)
	{
		int index = (i + TraceBufferIndex) % TraceBufferCount;
		printf(TraceSaveBuffer[index]);
	}
}

#define TRACE_UNDEFINED_BACKLOG DumpTraceBuffers()

#else // TRACE_BUFFER_ON_UNDEFINED

// Intended to be used in Cpu::Step
#define TRACE_INSTRUCTION(message) TRACE_INSTRUCTION_COMMON(message)
#define TRACE_SPRINTF sprintf

#endif // TRACE_BUFFER_ON_UNDEFINED

#else // TRACE_CPU_INSTRUCTIONS
#define TRACE_INSTRUCTION(message)
#define TRACE_SPRINTF ignore_args

static void ignore_args(...)
{

}

#endif // TRACE_CPU_INSTRUCTIONS

#ifndef TRACE_UNDEFINED_BACKLOG
#define TRACE_UNDEFINED_BACKLOG
#endif

#define TRACE_UNDEFINED(message) TRACE_UNDEFINED_BACKLOG; TRACE_INSTRUCTION_COMMON(message)

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
	Cycle = 0;
	Running = true;
	HandleInterrupt = false;
	RequestedInterrupts = 0;
}

unsigned short Cpu::InstructionPC()
{
	return SavedPC;
}

void Cpu::RequestIrq(int sourceFlag)
{
	RequestedInterrupts |= sourceFlag;
	CheckHandleInterrupt();
}
void Cpu::UnrequestIrq(int sourceFlag)
{
	RequestedInterrupts &= ~sourceFlag;
	CheckHandleInterrupt();
}



bool Cpu::Step()
{
	unsigned short instructionPC;
	unsigned char instruction;
	unsigned char temp, temp2;
	unsigned short stemp;
	char disasm[16];

	if (!Running)
	{
		return false;
	}

	if (HandleInterrupt)
	{
		// An interrupt was requested.
		// 1) Store PC to stack
		// 2) Store status register to stack
		// 3) Set interrupt disable bit in status register
		// 4) Load PC from FFFE
		// Then proceed normally.

		HandleInterrupt = false;
		Push(High(PC));
		Push(Low(PC));
		Push(P);
		SetFlag(IFlag);
		PC = Load16(0xFFFE);
	}

	instructionPC = PC;
	SavedPC = PC;

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

	case 0xA0: // LDY Immediate
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "LDY #$%02X", temp);
		TRACE_INSTRUCTION(disasm);
		Y = temp;
		SetResultFlags(Y);
		break;

	case 0xD0: //TRACE_INSTRUCTION("BNE");
		temp = LoadInstructionByte();
		stemp = PC + (char)temp;
		if ((P&ZFlag) == 0)
		{
			//temp = LoadInstructionByte();
			//PC += (char)temp; // Signed offset
			PC = stemp;
		}
		TRACE_SPRINTF(disasm, "BNE $%04X,X", stemp);
		TRACE_INSTRUCTION(disasm);
		break;

	case 0xE0: // Compare X
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "CPX #$%02X", temp);
		TRACE_INSTRUCTION(disasm);
		SetResultFlags(Sub(X, temp, 0));
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

	// 0x04 row

	case 0x24: // BIT (zeropage)
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "BIT $%02X", temp);
		TRACE_INSTRUCTION(disasm);
		SetResultFlags(A & Load(temp));
		break;

	case 0xA4: // Load Y
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "LDY $%02X", temp);
		TRACE_INSTRUCTION(disasm);
		Y = AttachedMemory->Read8(temp);
		SetResultFlags(Y);
		break;

	case 0xC4: // Compare Y (zeropage)
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "CPY $%02X", temp);
		TRACE_INSTRUCTION(disasm);
		SetResultFlags(Sub(Y, AttachedMemory->Read8(temp), 0));
		break;

	case 0xE4: // Compare X (zeropage)
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "CPX $%02X", temp);
		TRACE_INSTRUCTION(disasm);
		SetResultFlags(Sub(X, AttachedMemory->Read8(temp), 0));
		break;

	// 0x05 row

	case 0x05: // ORA (zeropage)
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "ORA $%02X", temp);
		TRACE_INSTRUCTION(disasm);
		A |= AttachedMemory->Read8(temp);
		SetResultFlags(A);
		break;

	case 0x45: // Exclusive OR (zeropage)
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "EOR $%02X", temp);
		TRACE_INSTRUCTION(disasm);
		A ^= Load(temp);
		SetResultFlags(A);
		break;

	case 0x65: // ADC
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "ADC $%02X", temp);
		TRACE_INSTRUCTION(disasm);
		A = Add(A, Load(temp), P & CFlag);
		SetResultFlags(A);
		break;

	case 0x85: // Store A
		temp = LoadInstructionByte();
		AttachedMemory->Write8(temp, A);
		TRACE_SPRINTF(disasm, "STA $%02X", temp);
		TRACE_INSTRUCTION(disasm);
		break;

	case 0xA5: // Load A
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "LDA $%02X", temp);
		TRACE_INSTRUCTION(disasm);
		A = Load(temp);
		SetResultFlags(A);
		break;

	case 0xC5: // Compare A
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "CMP $%02X", temp);
		TRACE_INSTRUCTION(disasm);
		SetResultFlags(Sub(A, AttachedMemory->Read8(temp), 0));
		break;

	case 0xE5: // Subtract with Carry (Zeropage)
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "SBC $%02X", temp);
		TRACE_INSTRUCTION(disasm);
		A = Sub(A, Load(temp), P & CFlag);
		SetResultFlags(A);
		break;

	// 0x06 row

	case 0x06: // Arithmetic Shift Left (zeropage)
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "ASL $%02X", temp);
		TRACE_INSTRUCTION(disasm);
		temp2 = Load(temp);
		if(temp2 & 0x80)
			P |= CFlag;
		else
			P &= ~CFlag;
		AttachedMemory->Write8(temp, temp2 << 1);
		SetResultFlags(temp2);
		break;

	case 0x46: // Logic Shift Right (Zeropage)
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "LSR $%02X", temp);
		TRACE_INSTRUCTION(disasm);
		temp2 = Load(temp);
		if(temp2 & 0x1)
			P |= CFlag;
		else
			P &= ~CFlag;
		AttachedMemory->Write8(temp, temp2 >> 1);
		SetResultFlags(temp2 >> 1);
		break;

	case 0x84: // Store Y (Zeropage)
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "STY $%02X", temp);
		TRACE_INSTRUCTION(disasm);
		AttachedMemory->Write8(temp, Y);
		break;

	case 0x86: // Store X (Zeropage)
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "STX $%02X", temp);
		TRACE_INSTRUCTION(disasm);
		AttachedMemory->Write8(temp, X);
		break;

	case 0xA6: // Load X (Zeropage)
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "LDX $%02X", temp);
		TRACE_INSTRUCTION(disasm);
		X = Load(temp);
		SetResultFlags(X);
		break;

	case 0xE6: // Increase (Zeropage)
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "INC $%02X", temp);
		TRACE_INSTRUCTION(disasm);
		temp2 = AttachedMemory->Read8(temp) + 1;
		AttachedMemory->Write8(temp, temp2);
		SetResultFlags(temp2);
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

	case 0x88: // Decrease Y
		TRACE_INSTRUCTION("DEY");
		Y--;
		SetResultFlags(Y);
		break;

	case 0xA8: // Transfer A to Y
		TRACE_INSTRUCTION("TAY");
		Y = A;
		SetResultFlags(Y);
		break;

	case 0xC8: // Increase Y
		TRACE_INSTRUCTION("INY");
		Y++;
		SetResultFlags(Y);
		break;

	case 0xE8: // Increase X
		TRACE_INSTRUCTION("INX");
		X++;
		SetResultFlags(X);
		break;

	// 0x09 row

	case 0x09: // OR on A
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "ORA #$%02X", temp);
		TRACE_INSTRUCTION(disasm);
		A = A | temp;
		SetResultFlags(A);
		break;

	case 0x29: // AND
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "AND #$%02X", temp);
		TRACE_INSTRUCTION(disasm);
		A = A & temp;
		SetResultFlags(A);
		break;

	case 0x49: // Exclusive OR
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "EOR #$%02X", temp);
		TRACE_INSTRUCTION(disasm);
		A ^= temp;
		SetResultFlags(A);
		break;

	case 0x69: // ADC
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "ADC #$%02X", temp);
		TRACE_INSTRUCTION(disasm);
		A = Add(A, temp, P & CFlag);
		SetResultFlags(A);
		break;

	case 0x89: TRACE_INSTRUCTION("NOP"); // Undocumented NOP
		break;

	case 0xA9: //Load Accumulator
		A = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "LDA #$%02X", A);
		TRACE_INSTRUCTION(disasm);
		break;

	case 0xC9: // Compare A (Immediate)
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "CMP #$%02X", temp);
		TRACE_INSTRUCTION(disasm);
		SetResultFlags(Sub(A, temp, 0));
		break;

	case 0xE9: // Subtract With Carry
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "SBC #$%02X", temp);
		TRACE_INSTRUCTION(disasm);
		A = Sub(A, temp, P & CFlag);
		SetResultFlags(A);
		break;

	// 0x0A row

	case 0x0A: // Arithmetic Shift Left
		TRACE_INSTRUCTION("ASL");
		if(A & 0x80)
			P |= CFlag;
		else
			P &= ~CFlag;
		A = A << 1;
		SetResultFlags(A);
		break;

	case 0x2A: // Rotate Left
		TRACE_INSTRUCTION("ROL");
		temp = A & 0x80;
		A = (A << 1) & 0xFF;
		if((P & CFlag) != 0)
			A |= 0x1;
		if(temp != 0)
			P |= CFlag;
		else
			P &= ~CFlag;
		SetResultFlags(A);
		break;

	case 0x8A: // Transfer X to A
		TRACE_INSTRUCTION("TXA");
		A = X;
		SetResultFlags(A);
		break;

	case 0xAA: // Transfer A to X
		TRACE_INSTRUCTION("TAX");
		X = A;
		SetResultFlags(X);
		break;

	case 0xCA: TRACE_INSTRUCTION("DEX"); // Decrement X
		X--;
		SetResultFlags(X);
		break;

	// 0x0C row

	case 0x2C: // Bit test (absolute)
		stemp = LoadInstructionShort();
		TRACE_SPRINTF(disasm, "BIT $%04X", stemp);
		TRACE_INSTRUCTION(disasm);
		SetResultFlags(A & stemp);
		break;

	case 0x4C: // Jump (direct)
		stemp = LoadInstructionShort();
		TRACE_SPRINTF(disasm, "JMP $%04X", stemp);
		TRACE_INSTRUCTION(disasm);
		PC = stemp;
		break;

	case 0x6C: // Jump (indirect)
		stemp = LoadInstructionShort();
		TRACE_SPRINTF(disasm, "JMP ($%04X)", stemp);
		TRACE_INSTRUCTION(disasm);
		PC = Load16(stemp);
		break;

	case 0x8C: // Store Y
		stemp = LoadInstructionShort();
		TRACE_SPRINTF(disasm, "STY $%04X", stemp);
		TRACE_INSTRUCTION(disasm);
		AttachedMemory->Write8(stemp, Y);
		break;

	case 0xAC: // Load Y
		stemp = LoadInstructionShort();
		TRACE_SPRINTF(disasm, "LDY $%04X", stemp);
		TRACE_INSTRUCTION(disasm);
		Y = Load16(stemp);
		SetResultFlags(Y);
		break;

	// 0x0D row

	case 0x0D: // Or A
		stemp = LoadInstructionShort();
		TRACE_SPRINTF(disasm, "ORA $%04X", stemp);
		TRACE_INSTRUCTION(disasm);
		A |= Load(stemp);
		SetResultFlags(A);
		break;

	case 0x8D: // Store A
		stemp = LoadInstructionShort();
		AttachedMemory->Write8(stemp, A);
		TRACE_SPRINTF(disasm, "STA $%04X", stemp);
		TRACE_INSTRUCTION(disasm);
		break;

	case 0xAD: // Load A
		stemp = LoadInstructionShort();
		TRACE_SPRINTF(disasm, "LDA $%04X", stemp);
		TRACE_INSTRUCTION(disasm);
		A = Load(stemp);
		SetResultFlags(A);
		break;

	case 0xCD: // Compare A (Absolute)
		stemp = LoadInstructionShort();
		TRACE_SPRINTF(disasm, "CMP $%04X", stemp);
		TRACE_INSTRUCTION(disasm);
		SetResultFlags(Sub(A, Load(stemp), 0));
		break;

	// 0x0E row
	case 0x8E: // Store X
		stemp = LoadInstructionShort();
		AttachedMemory->Write8(stemp, X);
		TRACE_SPRINTF(disasm, "STX $%04X", stemp);
		TRACE_INSTRUCTION(disasm);
		break;

	case 0xAE: // Load X
		stemp = LoadInstructionShort();
		TRACE_SPRINTF(disasm, "LDX $%04X", stemp);
		TRACE_INSTRUCTION(disasm);
		X = Load16(stemp);
		SetResultFlags(X);
		break;

	// 0x10 row

	case 0x10: // Branch if Plus
		stemp = (char)LoadInstructionByte();
		stemp += PC;
		TRACE_SPRINTF(disasm, "BPL $%04X", stemp);
		TRACE_INSTRUCTION(disasm);
		if(!(P & NFlag))
			PC = stemp;
		break;

	case 0x30: // Branch if Minus
		stemp = (char)LoadInstructionByte();
		stemp += PC;
		TRACE_SPRINTF(disasm, "BMI $%04X", stemp);
		TRACE_INSTRUCTION(disasm);
		if(P & NFlag)
			PC = stemp;
		break;

	case 0x90: // Branch if Carry is Clear
		temp = LoadInstructionByte();
		stemp = PC + temp;
		TRACE_SPRINTF(disasm, "BCC $%04X", stemp);
		TRACE_INSTRUCTION(disasm);
		if(!(P & CFlag))
			PC = stemp;
		break;

	case 0xB0: // Branch if Carry Set
		stemp = (char)LoadInstructionByte();
		stemp += PC;
		TRACE_SPRINTF(disasm, "BCS $%04X", stemp);
		TRACE_INSTRUCTION(disasm);
		if(P & CFlag)
			PC = stemp;
		break;

	case 0xF0: // Branch if Equal
		stemp = (char)LoadInstructionByte();
		stemp += PC;
		TRACE_SPRINTF(disasm, "BEQ $%04X", stemp);
		TRACE_INSTRUCTION(disasm);
		if ( (P&ZFlag) == ZFlag)
			PC = stemp;
		break;

	// 0x11 row

	case 0x71: // Add with Carry (Indirect-indexed)
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "ADC ($%02X),Y", temp);
		TRACE_INSTRUCTION(disasm);
		A = Add(A, Load(Load16(temp) + Y), P & CFlag);
		SetResultFlags(A);
		break;

	case 0x91: // Store A (Indirect-indexed)
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "STA ($%02X),Y", temp);
		TRACE_INSTRUCTION(disasm);
		//AttachedMemory->Write8(Load16(temp) + Y, A);
		AttachedMemory->Write8(Load16(temp) + Y, A);
		break;

	case 0xB1: // Load A (Indirect-indexed)
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "LDA ($%02X),Y", temp);
		TRACE_INSTRUCTION(disasm);
		//A = Load(Load16(temp) + Y);
		A = Load(Load16(temp) + Y);
		SetResultFlags(A);
		break;

	case 0xD1: // Compare A (Indirect-indexed)
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "CMP ($%02X),Y", temp);
		TRACE_INSTRUCTION(disasm);
		Sub(A, Load(Load16(temp) + Y), 0);
		break;

	// 0x14 row

	case 0x94: // Store Y
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "STY $%02X,X", temp);
		TRACE_INSTRUCTION(disasm);
		AttachedMemory->Write8(temp + X, Y);
		break;

	case 0xB4: // Load Y
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "LDY $%02X,X", temp);
		TRACE_INSTRUCTION(disasm);
		Y = Load(temp + X);
		SetResultFlags(Y);
		break;

	// 0x15 row

	case 0x95: // Store A
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "STA $%02X,X", temp);
		TRACE_INSTRUCTION(disasm);
		AttachedMemory->Write8(temp + X, A);
		break;

	case 0xB5: // Load A
		temp = LoadInstructionByte();
		TRACE_SPRINTF(disasm, "LDA $%02X,X", temp);
		TRACE_INSTRUCTION(disasm);
		A = AttachedMemory->Read8(temp + X);
		SetResultFlags(A);
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
		CheckHandleInterrupt();
		break;

	case 0x78: TRACE_INSTRUCTION("SEI");
		SetFlag(IFlag);
		CheckHandleInterrupt();
		break;

	case 0x98: // Transfer Y to A
		TRACE_INSTRUCTION("TYA");
		A = Y;
		SetResultFlags(A);
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

	// 0x19 row

	case 0x99:
		stemp = LoadInstructionShort();
		TRACE_SPRINTF(disasm, "STA $%04X,Y", stemp);
		TRACE_INSTRUCTION(disasm);
		AttachedMemory->Write8(stemp + Y, A);
		break;

	case 0xB9: // Load A
		stemp = LoadInstructionShort();
		TRACE_SPRINTF(disasm, "LDA $%04X,Y", stemp);
		TRACE_INSTRUCTION(disasm);
		A = AttachedMemory->Read8(stemp + Y);
		SetResultFlags(A);
		break;

	// 0x1A row 

	case 0x9A: TRACE_INSTRUCTION("TXS"); // Transfer X to Stack register
		S = X;
		break;

	// 0x1D row (Absolute, X)

	case 0x9D: // Store A
		stemp = LoadInstructionShort();
		TRACE_SPRINTF(disasm, "STA $%04X,X", stemp);
		TRACE_INSTRUCTION(disasm);
		AttachedMemory->Write8(stemp + X, A);
		break;

	case 0xBD: //TRACE_INSTRUCTION("LDA"); // Load a from [Immediate16+X]
		stemp = LoadInstructionShort();
		//A = Load(LoadInstructionShort() + X);
		//A = Load(stemp) + X;
		A = Load(stemp +X);
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
		Running = false;
		return false; // CPU does not recognize this instruction.

	}


	Cycle += 2; // All instructions take two cycles at least. Additional cycles are often due to memory access.

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

void Cpu::CheckHandleInterrupt()
{
	if (P & IFlag)
	{
		// Interrupts are suppressed.
		HandleInterrupt = false;
	}
	else
	{
		// Interrupts are enabled.
		if (RequestedInterrupts != 0)
		{
			HandleInterrupt = true;
		}
	}
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
