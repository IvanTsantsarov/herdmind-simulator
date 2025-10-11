#ifndef TOOLS_H
#define TOOLS_H

#include <cstdint>


class Tools
{

public:
    Tools();
    static int16_t f2i16(float v, float scale);
    static float rnd(float minv, float maxv);
};

#endif // TOOLS_H
