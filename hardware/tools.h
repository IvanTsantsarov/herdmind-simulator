#ifndef TOOLS_H
#define TOOLS_H

#include <cstdint>

class Tools
{

public:

    Tools();
    static int16_t f2i16(float v, float scale);
    static float rnd(float minv, float maxv);

    // Geo location
    static uint32_t encodeLon(double lonDeg);
    static double decodeLon(uint32_t u);
    static uint32_t encodeLat(double latDeg);
    static double decodeLat(uint32_t u);
};

#endif // TOOLS_H
