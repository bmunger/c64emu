
#include "Cpu.h"
#include "Memory.h"
#include "Video.h"
#include "Keyboard.h"
#include "Emulation.h"
#include <stdio.h>

#define TRACE_IO_ACCESS 1

static void ignore_args(...)
{

}

#if TRACE_IO_ACCESS
#define PRINT_IO printf
#else
#define PRINT_IO ignore_args
#endif

CIAChip::CIAChip(int CpuInterruptSourceIndex) : evtTimerA(CallbackTimerA, this), evtTimerB(CallbackTimerB, this)
{
	InterruptSourceIndex = CpuInterruptSourceIndex;
}

void CIAChip::Setup(Memory* useMemory, FnPtrCiaCallback readFn, FnPtrCiaCallback writeFn)
{
	AttachedMemory = useMemory;
	CbRead = readFn;
	CbWrite = writeFn;
}

void CIAChip::Reset()
{
	PRA = PRB = 0;
	DDRA = DDRB = 0;
	TAValue = TBValue = 0;
	TALatch = TBLatch = 0;
	CRA = CRB = 0;
	IntFlags = 0;
	IntMask = 0;
	MaskedFlags = 0;
}


void CIAChip::Write8(int Address, unsigned char Data8)
{
	switch (Address & 15)
	{
	case 0: // PRA
		PrevPRA = PRA; PrevPRB = PRB;
		PRA = Data8;
		if (CbWrite) CbWrite(this);
		break;
	case 1: // PRB
		PrevPRA = PRA; PrevPRB = PRB;
		PRB = Data8;
		if (CbWrite) CbWrite(this);
		break;
	case 2: // DDRA
		DDRA = Data8;
		break;
	case 3: // DDRB
		DDRB = Data8;
		break;
	case 4: // TA Lo
		TALatch = (TALatch & 0xFF00) | Data8;
		break;
	case 5: // TA Hi
		TALatch = (TALatch & 0xFF) | (Data8 << 8);
		if ((CRA & CR_START) == 0)
		{
			TAValue = TALatch;
		}
		break;
	case 6: // TB Lo
		TBLatch = (TBLatch & 0xFF00) | Data8;
		break;
	case 7: // TB Hi
		TBLatch = (TBLatch & 0xFF) | (Data8 << 8);
		if ((CRB & CR_START) == 0)
		{
			TBValue = TBLatch;
		}
		break;
	case 8: // TOD 10ths
	case 9: // TOD Sec
	case 10: // TOD Min
	case 11: // TOD Hr
	case 12: // SDR
		break;
	case 13: // ICR
		if (Data8 & 0x80)
		{
			// Set interrupt mask bits
			IntMask |= (Data8 & 0x1F);
		}
		else
		{
			// Clear interrupt mask bits
			IntMask &= ~(Data8 & 0x1F);
		}
		UpdateInterruptStatus();
		break;

	case 14: // CRA
		// If timer was running, Update its value.
		if (CRA & CR_START)
		{
			UpdateTimerA();
		}

		if (Data8 & CR_LOAD)
		{
			TAValue = TALatch;
		}

		CRA = Data8 & (~CR_LOAD);
		StartTimerA();
		break;

	case 15: // CRB
		// If timer was running, Update its value.
		if (CRA & CR_START)
		{
			UpdateTimerA();
		}

		if (Data8 & CR_LOAD)
		{
			TAValue = TALatch;
		}

		CRA = Data8 & (~CR_LOAD);
		StartTimerA();
		break;
	}
}
unsigned char CIAChip::Read8(int Address)
{
	switch (Address & 15)
	{
	case 0: // PRA
		PrevPRA = PRA; PrevPRB = PRB;
		// Any unconnected bits by default float up to 1.
		PRA |= ~DDRA;
		if (CbRead) CbRead(this);
		return PRA;
	case 1: // PRB
		PrevPRA = PRA; PrevPRB = PRB;
		// Any unconnected bits by default float up to 1.
		PRB |= ~DDRB;
		if (CbRead) CbRead(this);
		return PRB;
	case 2: // DDRA
		return DDRA;
	case 3: // DDRB
		return DDRB;
	case 4: // TA Lo
		UpdateTimerA();
		return TAValue & 0xFF;
	case 5: // TA Hi
		UpdateTimerA();
		return TAValue >> 8;
	case 6: // TB Lo
		UpdateTimerB();
		return TBValue & 0xFF;
	case 7: // TB Hi
		UpdateTimerB();
		return TBValue >> 8;
	case 8: // TOD 10ths
	case 9: // TOD Sec
	case 10: // TOD Min
	case 11: // TOD Hr
	case 12: // SDR
		break;
	case 13: // ICR
	{
		unsigned char result = IntFlags;
		// Reading this register clears the interrupt flags (stops pending interrupts)
		IntFlags = 0;
		UpdateInterruptStatus();
		return result;
	}
	case 14: // CRA
		return CRA;
	case 15: // CRB
		return CRB;
	}
	return 0xFF; // unimplemented.
}

void CIAChip::SetIntFlags(int flags)
{
	IntFlags |= flags;
	UpdateInterruptStatus();
}
void CIAChip::ClearIntFlags(int flags)
{
	IntFlags &= (~flags);
	UpdateInterruptStatus();
}
void CIAChip::UpdateInterruptStatus()
{
	int newMaskedFlags = IntFlags & IntMask;
	if (newMaskedFlags != 0 && MaskedFlags == 0)
	{
		// Interrupt flag has been raised!
		AttachedMemory->AttachedCpu->RequestIrq(InterruptSourceIndex);
	}
	if (newMaskedFlags == 0 && MaskedFlags != 0)
	{
		// Interrupt status has been cleared
		AttachedMemory->AttachedCpu->UnrequestIrq(InterruptSourceIndex);
	}
	if (newMaskedFlags != 0)
	{
		// Mark interrupt flag to show interrupt has been requested.
		IntFlags |= 0x80;
	}
	MaskedFlags = newMaskedFlags;
}

void CIAChip::UpdateTimerA()
{
	if (CRA & CR_START)
	{
		if ((CRA & CRA_INMODE) == 0) // use CLK for counting.
		{
			long long curCycle = AttachedMemory->AttachedCpu->Cycle;
			int ElapsedCycles = curCycle - LastEventA;
			AdvanceTimerA(ElapsedCycles);
			LastEventA = curCycle;
		}
	}
}
void CIAChip::UpdateTimerB()
{
	if (CRB & CR_START)
	{
		if ((CRB & CRB_INMODE_MASK) == CRB_INMODE_CLK) // use CLK for counting.
		{
			long long curCycle = AttachedMemory->AttachedCpu->Cycle;
			int ElapsedCycles = curCycle - LastEventB;

		}
	}
}

void CIAChip::AdvanceTimerA(int counts)
{
	if (counts > TAValue)
	{
		SetIntFlags(1); // Timer A interrupt, underflow.

		if ((CRA & CR_RUNMODE) == 1)
		{
			// One-shot. Timer now reloads the latch value and stops.
			TAValue = TALatch;
			CRA &= ~(CR_START);
			TimerAUnderflow(1);
		}
		else
		{
			// Continuous mode. Compute a new TAValue and proceed.
			counts -= TAValue + 1; // How many additional cycles after the interrupt
			int underflowcount = 1 + counts / (TALatch + 1);
			TimerAUnderflow(underflowcount);

			counts = counts % (TALatch + 1); // If it overflowed multiple times (unlikely) subtract multiples of the latch value.
			TAValue = TALatch - counts; // New value accurate to the current cycle.
		}
	}
	else
	{
		TAValue -= counts;
	}
}
void CIAChip::AdvanceTimerB(int counts)
{
	if (counts > TBValue)
	{
		SetIntFlags(1); // Timer A interrupt, underflow.

		if ((CRB & CR_RUNMODE) == 1)
		{
			// One-shot. Timer now reloads the latch value and stops.
			TBValue = TBLatch;
			CRB &= ~(CR_START);
		}
		else
		{
			// Continuous mode. Compute a new Value and proceed.
			counts -= TBValue + 1; // How many additional cycles after the interrupt
			counts = counts % (TBLatch + 1); // If it overflowed multiple times (unlikely) subtract multiples of the latch value.
			TBValue = TBLatch - counts; // New value accurate to the current cycle.
		}
	}
	else
	{
		TBValue -= counts;
	}
}



void CIAChip::StartTimerA()
{
	if (CRA & CR_START)
	{
		if ((CRA & CRA_INMODE) == 0) // use CLK for counting
		{
			LastEventA = AttachedMemory->AttachedCpu->Cycle;

			// Set callback for the time in the future when this timer will underflow.
			long long underflowCycle = LastEventA + TAValue + 1;
			AttachedMemory->AttachedEmulation->QueueEvent(underflowCycle, &evtTimerA);
			return;
		}
	}
	// Timer isn't running, remove any callback.
	AttachedMemory->AttachedEmulation->CancelEvent(&evtTimerA);
}
void CIAChip::StartTimerB()
{
	if (CRB & CR_START)
	{
		if ((CRB & CRB_INMODE_MASK) == CRB_INMODE_CLK)
		{
			LastEventA = AttachedMemory->AttachedCpu->Cycle;

			// Set callback for the time in the future when this timer will underflow.
			long long underflowCycle = LastEventB + TBValue + 1;
			AttachedMemory->AttachedEmulation->QueueEvent(underflowCycle, &evtTimerB);
			return;
		}
	}
	// Timer isn't running, remove any callback.
	AttachedMemory->AttachedEmulation->CancelEvent(&evtTimerB);
}

void CIAChip::TimerAUnderflow(int count)
{
	// If timer B is counting pulses from timer A, advance timer B.
	if (CRB & CR_START)
	{
		if ((CRB & CRB_INMODE_MASK) == CRB_INMODE_TA)
		{
			AdvanceTimerB(count);
		}
	}
}


void CIAChip::CallbackTimerA(EventRequest* Request)
{
	CIAChip* chip = (CIAChip*)Request->Context;
	chip->UpdateTimerA();
	chip->StartTimerA();
}
void CIAChip::CallbackTimerB(EventRequest* Request)
{
	CIAChip* chip = (CIAChip*)Request->Context;
	chip->UpdateTimerB();
	chip->StartTimerB();
}






Memory::Memory() : RAM(nullptr), Kernal(nullptr), Basic(nullptr), Char(nullptr), CIA1(InterruptSourceCIA1), CIA2(InterruptSourceCIA2)
{
	RAM = new unsigned char[65536];

	Kernal = LoadRom("roms/901227-03.u4", 8192);
	Basic = LoadRom("roms/901226-01.u3", 8192);
	Char = LoadRom("roms/901225-01.u5", 4096);

	CIA1.Setup(this, Cia1Read, Cia1Write);
	CIA2.Setup(this, Cia2Read, Cia2Write);

	// Todo: Load rom images from file.
	Reset();
}


Memory::~Memory()
{
	delete[] RAM;
	delete[] Kernal;
	delete[] Basic;
	delete[] Char;
}

void Memory::Reset()
{
	// 7..0 bits are (unused) (unused) (Casette Motor, 0=on) (Casette Sense 0=playing) (Casette data) (CHAREN) (HIRAM) (LORAM)
	// Mapped segments are: E000-FFFF Kernal ROM, D000-DFFF is Char and IO space, A000-BFFF is BASIC
	// HIRAM: 1=Kernal ROM and Basic ROM present. 0 = Neither present
	// LORAM: 1 = Basic rom present if HIRAM = 1, 0 = not present
	// if HIRAM and LORAM are both 0, IO and Char disappear also.
	// CHAREN: 1 = IO space is visible, 0 = Char space is visible.
	DDR = 0x2F;
	PR = 0x37;

	CIA1.Reset();
	CIA2.Reset();
}

unsigned char Memory::EffectivePR()
{
	// If bits in DDR are set to input (0), the values will read as '1' unless externally driven.
	return (PR | (~DDR)) & 0x3F;
}

void Memory::Write8(int Address, unsigned char Data8)
{
	int tempPR = 0;
	if (Address < 0 || Address > 65535)
	{
		printf("Write8: Address out of bounds %04X %02X\n", Address, Data8);
		return;
	}


	// Side effects
	if (Address == 0)
	{
		DDR = Data8;
	}
	else if (Address == 1)
	{
		PR = Data8;
	}
	else if (Address >= 0xD000 && Address < 0xE000) // IO / Char space
	{
		// IO or Character ram.
		tempPR = EffectivePR();
		if ((tempPR & (HIRAM | LORAM)) != 0)
		{
			// IO/Char memory space is not disabled
			PRINT_IO("IO Write 0x%02X => [%04X] (%lld, PC=%04X)\n", Data8, Address, AttachedCpu->Cycle, AttachedCpu->InstructionPC());
			if (tempPR & CHAREN)
			{
				// Write to I/O memory
				if (Address >= 0xD400 && Address < 0xD800)
				{
					// SID range
				}
				else if (Address >= 0xDE00)
				{
					// External I/O area
				}
				else if (Address >= 0xDD00)
				{
					// CIA 2
					CIA2.Write8(Address, Data8);
				}
				else if (Address >= 0xDC00)
				{
					// CIA 1
					CIA1.Write8(Address, Data8);
				}
				else
				{
					// VIC-II I/O
					AttachedVideo->Write8(Address, Data8);
				}
				
				// (Not entirely certain if this also writes to RAM. I think not.)
				return;
			}
		}
	}

	// Write to memory unless above block suppressed it.
	RAM[Address] = Data8;
}

unsigned char Memory::Read8(int Address)
{
	int tempPR = 0;
	if (Address < 0 || Address > 65535)
	{
		printf("Read8: Address out of bounds %04X\n", Address);
		return 0xFF;
	}

	if (Address == 0)
	{
		return DDR;
	}
	else if (Address == 1)
	{
		return EffectivePR();
	}
	else if (Address >= 0xE000) // Kernal space
	{
		tempPR = EffectivePR();
		if ((tempPR & HIRAM) != 0)
		{
			// Kernal is enabled
			return Kernal[Address - 0xE000];
		}

	}
	else if (Address >= 0xD000 && Address < 0xE000) // IO / Char space
	{
		tempPR = EffectivePR();
		if ((tempPR & (HIRAM | LORAM)) != 0)
		{
			// IO/Char memory space is not disabled

			if (tempPR & CHAREN)
			{
				unsigned char IORead = 0xFF;
				// This is I/O memory
				if (Address >= 0xD400 && Address < 0xD800)
				{
					// SID range
				}
				else if (Address >= 0xDE00)
				{
					// External I/O area
				}
				else if (Address >= 0xDD00)
				{
					// CIA 2
					IORead = CIA2.Read8(Address);
				}
				else if (Address >= 0xDC00)
				{
					// CIA 1
					IORead = CIA1.Read8(Address);
				}
				else
				{
					// VIC-II I/O
					IORead = AttachedVideo->Read8(Address);
				}

				PRINT_IO("IO Read [%04X] => 0x%02X (%lld, PC=%04X)\n", Address, IORead, AttachedCpu->Cycle, AttachedCpu->InstructionPC());

				return IORead;
			}
			else
			{
				// This is Char memory
				return Char[Address - 0xD000];
			}
		}
	}
	else if (Address >= 0xA000 && Address < 0xC000) // Basic space
	{
		tempPR = EffectivePR();
		if ((tempPR & (HIRAM | LORAM)) == (HIRAM | LORAM))
		{
			// Basic space is enabled.
			return Basic[Address - 0xA000];
		}
	}

	// Todo: ROM emulation & IO space


	return RAM[Address];
}


unsigned char * Memory::LoadRom(const char * Filename, int Size)
{
	// This may crash if it fails. Replace later.
	FILE* f = fopen(Filename, "rb");
	unsigned char * data = new unsigned char[Size];

	fread(data, Size, 1, f);

	fclose(f);

	return data;
}

void Memory::Cia1Read(CIAChip* chip)
{
	chip->AttachedMemory->AttachedKeyboard->UpdateKeyboardMatrix(chip->PRA, chip->PRB, chip->DDRA, chip->DDRB);
}
void Memory::Cia1Write(CIAChip* chip)
{

}
void Memory::Cia2Read(CIAChip* chip)
{

}
void Memory::Cia2Write(CIAChip* chip)
{

}
