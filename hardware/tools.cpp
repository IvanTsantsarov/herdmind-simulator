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

uint32_t Tools::encodeLon(double lonDeg)
{
    lonDeg = std::clamp(lonDeg, -180.0, 180.0);
    // map [-180, 180] -> [0, 2^32-1]
    long double u = ( (long double)(lonDeg + 180.0L) / 360.0L ) * 4294967295.0L;
    return (uint32_t) llround(u);
}

double Tools::decodeLon(uint32_t u)
{
    // map [0, 2^32-1] -> [-180, 180]
    long double lon = ((long double)u / 4294967295.0L) * 360.0L - 180.0L;
    return (double)lon;
}

uint32_t Tools::encodeLat(double latDeg)
{
    latDeg = std::clamp(latDeg, -90.0, 90.0);
    // map [-90, 90] -> [0, 2^32-1]
    long double u = ( (long double)(latDeg + 90.0L) / 180.0L ) * 4294967295.0L;
    return (uint32_t) llround(u);
}

double Tools::decodeLat(uint32_t u) {
    long double lat = ((long double)u / 4294967295.0L) * 180.0L - 90.0L;
    return (double)lat;
}
