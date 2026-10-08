#ifndef MEMORY_H
#define MEMORY_H

#include "../arduino.h"

#define MEMORY_MAX_BUFFER 100

class Memory
{
    char* mBuffer = nullptr;
    int mBufferLen = 0;

public:
    Memory();
    size_t read(const char* name, char *dst, size_t maxLen);
    size_t write(const char *name, const char *val, size_t len);
    bool readKey(const char* name, uint8_t* dst);
    bool writeKey(const char* name, const uint8_t *src);
};

#endif // MEMORY_H
