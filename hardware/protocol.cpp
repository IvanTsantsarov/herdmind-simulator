#include <cmath>
#include "protocol.h"

int16_t Protocol::f2i16(float v, float scale) {
    long s = lroundf(v * scale);
    if (s > 32767) s = 32767;
    if (s < -32768) s = -32768;
    return (int16_t) s;
}

Protocol::Protocol() {}

Protocol::Error Protocol::parseDevice(void *data, uint32_t size, uint32_t& parsedBytes )
{
    if( size > MAX_PACKAGE_SIZE ) {
        return Error::TooBig;
    }

    if( size < static_cast<int>(sizeof(ServerHead)) ) {
        return Error::SmallerThenHeader;
    }



    return Error::NoError;
}
