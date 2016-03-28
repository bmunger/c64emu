#include "Memory.h"
#include "Video.h"
#include <stdio.h>

Memory::Memory()
{
	RAM = new unsigned char[65536];
	Kernal = new unsigned char[8192];
	Basic = new unsigned char[8192];

	// Todo: Load rom images from file.

	Reset();
}


Memory::~Memory()
{
}

void Memory::Reset()
{
	// 7..0 bits are (unused) (unused) (Casette Motor, 0=on) (Casette Sense 0=playing) (Casette data) (CHAREN) (HIRAM) (LORAM)
	// HIRAM: 1=Kernal ROM present from E000-FFFF, 0=it's ram
	// Not entirely sure how the other bits work yet.
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
	else if (Address >= 0xD000 && Address < 0xE000)
	{
		// IO or Character ram.


	}

	// Write to memory unless above block suppressed it.
	RAM[Address] = Data8;
}

unsigned char Memory::Read8(int Address)
{
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
	// Todo: ROM emulation & IO space


	return RAM[Address];
}
