#ifndef _EMULATION_H
#define _EMULATION_H

#include "Video.h"
#include "Memory.h"
#include "Cpu.h"

class Emulation
{
public:
    Emulation();
    ~Emulation();

    void Reset();
    void Run();

    Video SystemVideo;
    Memory SystemMemory;
    Cpu SystemCpu;
};

#endif