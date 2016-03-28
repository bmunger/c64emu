#ifndef _VIDEO_H
#define _VIDEO_H

class Memory;
class Cpu;

class Video
{
public:
    Video();
    ~Video();

    void Reset();
    void VideoStep();

    Memory * AttachedMemory;
    Cpu * AttachedCpu;
};

#endif