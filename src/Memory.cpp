#include "Memory.h"
#include "Video.h"
#include <stdio.h>

Memory::Memory() : RAM(nullptr), Kernal(nullptr), Basic(nullptr), Char(nullptr)
{
	RAM = new unsigned char[65536];

	Kernal = LoadRom("roms/901227-03.u4", 8192);
	Basic = LoadRom("roms/901226-01.u3", 8192);
	Char = LoadRom("roms/901225-01.u5", 4096);

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
				}
				else if (Address >= 0xDC00)
				{
					// CIA 1
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
				}
				else if (Address >= 0xDC00)
				{
					// CIA 1
				}
				else
				{
					// VIC-II I/O
					return AttachedVideo->Read8(Address);
				}

				return 0xFF;
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