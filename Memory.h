#ifndef _MEMORY_H
#define _MEMORY_H

class Video;

class Memory
{
public:
    Memory();
    ~Memory();

    void Reset();

    void Write8(int Address, unsigned char Data8);
    unsigned char Read8(int Address);

    Video * AttachedVideo;
};

#endif