#ifndef EXTERN_H
#define EXTERN_H

#ifdef SIMULATION
//////////////////////////////////////////////////////////////
/// Simulation
//////////////////////////////////////////////////////////////
inline
bool extReadTemperature(float& t)
{

    return false;
}

inline
bool extReadAcceleration(float& ax, float& ay, float& az)
{

    return false;
}

inline
bool extSendData(const char* data, int size)
{

    return false;
}
#else
//////////////////////////////////////////////////////////////
/// Real conditions
//////////////////////////////////////////////////////////////
inline
    bool extReadTemperature(float& t) {

    return false;
}

inline
    bool extReadAcceleration(float& ax, float& ay, float& az) {

    return false;
}

inline
    bool extSendData(const char* data, int size) {

    return false;
}

#endif // SIMULATION

#endif // EXTERN_H
