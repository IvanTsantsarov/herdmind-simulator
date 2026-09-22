#ifndef MEMORY_H
#define MEMORY_H

#ifdef SIMULATION
    #include "../tools.h"
#else
    #include <Arduino.h>
#endif

#define MEMORY_MAX_BUFFER 100

class Memory
{
    char* mBuffer = nullptr;
    int mBufferLen = 0;
public:
    Memory();
    void readBegin();
    void writeBegin();
    char* read(const char* name);
    void write(const char *name, const String& val);
    void end();
};

#endif // MEMORY_H
