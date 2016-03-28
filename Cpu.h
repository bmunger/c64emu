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
};

#endif