#include <cmath>
#include <algorithm>
#include "tools.h"

Tools::Tools() {}


int16_t Tools::f2i16(float v, float scale) {
    long s = lroundf(v * scale);
    if (s > 32767) s = 32767;
    if (s < -32768) s = -32768;
    return (int16_t) s;
}


float Tools::rnd(float minv, float maxv)
{
    return minv + (maxv - minv) * (float)rand() * (1.0f / (float)RAND_MAX);
}

