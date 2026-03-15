#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <ctime>

#define PROTOCOL_VERSION 1

#define UNIX_TO_2000 946684800

class Protocol
{
    inline static uint32_t timestamp()
    {
        auto tm = std::time(nullptr);
        return (uint32_t)(tm - UNIX_TO_2000);
    }

    inline static uint32_t readUint32(const uint8_t* array, int offset) {
        return  (static_cast<uint32_t>(array[offset])   << 24)|
                (static_cast<uint32_t>(array[offset+1]) << 16) |
                (static_cast<uint32_t>(array[offset+2]) << 8) |
                (static_cast<uint32_t>(array[offset+3]));

    }
    inline static void writeUint32(uint32_t value, uint8_t* array, int offset) {
        array[offset]   = (value & 0xFF000000) >> 24;
        array[offset+1] = (value & 0x00FF0000) >> 16;
        array[offset+2] = (value & 0x0000FF00) >> 8;
        array[offset+3] =  value & 0x000000FF ;



    }
public:

    struct BolusByteArray;

    struct Bolus {

        enum Condition {
            Normal = 0,
            Unavailable = 1,
            Atony = 2,
            Hypomotility = 4,
            Hyperactivity = 8,
            HighTemperature = 16
        };

        inline bool hasCondition(Condition condition) {
            return mCondition & condition;
        }

        inline bool isOk(){ return Condition::Normal == mCondition; }
        inline bool isAvailable(){ return !hasCondition(Condition::Unavailable); }
        inline bool hasAtony(){ return hasCondition(Condition::Atony); }
        inline bool hasHypo(){ return hasCondition(Condition::Hypomotility); }
        inline bool hasHiper(){ return hasCondition(Condition::Hyperactivity); }
        inline bool hasTemp(){ return hasCondition(Condition::HighTemperature); }

        uint32_t mTimestamp = timestamp();  // unix timestamp
        uint8_t mCondition = 0;      // see bolus.h
        uint8_t mRssi = 0;       // signal level
        uint8_t mBattery = 0;        // mBattery level


        inline void fromByteArray(const uint8_t* array) {
            mTimestamp = Protocol::readUint32(array, 0);
            mCondition = array[sizeof(uint32_t)];
            mRssi      = array[sizeof(uint32_t)+1];
            mBattery   = array[sizeof(uint32_t)+2];
        }

        inline BolusByteArray toByteArray() const {
            BolusByteArray ba;
            Protocol::writeUint32(mTimestamp, ba, 0);
            ba[sizeof(uint32_t)]  = mCondition;
            ba[sizeof(uint32_t)+1]= mRssi;
            ba[sizeof(uint32_t)+2]= mBattery;
            return ba;
        }
    };

    struct CollarByteArray;

    struct Collar {
        enum struct Event : uint8_t {
            None = 0,
            Light = 10,
            Sound = 11,
            Shock = 12
        };

        uint32_t mTimestamp = timestamp();  // unix timestamp
        uint32_t mLongitude = 0;  // positioning mLongitude
        uint32_t mLatitude = 0;   // positioning mLatitude
        Event mEvent = Event::None;       // mEvent type level
        uint8_t mRssi = 0;        // signal level
        uint8_t mBattery = 0;     // mBattery level

        inline void fromByteArray(const uint8_t* array) {
            mTimestamp = Protocol::readUint32(array, 0);
            mLongitude = Protocol::readUint32(array,   sizeof(uint32_t));
            mLatitude  = Protocol::readUint32(array, 2*sizeof(uint32_t));
            mEvent   = (Event)array[ 3*sizeof(uint32_t)];
            mRssi    = array[ 3*sizeof(uint32_t) + 1];
            mBattery = array[ 3*sizeof(uint32_t) + 2];
        }

        inline CollarByteArray toByteArray()  const {
            CollarByteArray ba;
            Protocol::writeUint32(mTimestamp, ba, 0);
            Protocol::writeUint32(mLongitude, ba,   sizeof(uint32_t));
            Protocol::writeUint32(mLatitude,  ba, 2*sizeof(uint32_t));
            ba[3*sizeof(uint32_t)]   = (uint8_t)mEvent;
            ba[3*sizeof(uint32_t)+1] = mRssi;
            ba[3*sizeof(uint32_t)+2] = mBattery;
            return ba;
        }

        inline void encodeLon(double lonDeg)
        {
            lonDeg = std::clamp(lonDeg, -180.0, 180.0);
            // map [-180, 180] -> [0, 2^32-1]
            long double u = ( (long double)(lonDeg + 180.0L) / 360.0L ) * 4294967295.0L;
            mLongitude = (uint32_t) llround(u);
        }

        inline double decodeLon()
        {
            // map [0, 2^32-1] -> [-180, 180]
            long double lon = ((long double)mLongitude / 4294967295.0L) * 360.0L - 180.0L;
            return (double)lon;
        }

        inline void encodeLat(double latDeg)
        {
            latDeg = std::clamp(latDeg, -90.0, 90.0);
            // map [-90, 90] -> [0, 2^32-1]
            long double u = ( (long double)(latDeg + 90.0L) / 180.0L ) * 4294967295.0L;
            mLatitude = (uint32_t) llround(u);
        }

        inline double decodeLat() {
            long double lat = ((long double)mLatitude / 4294967295.0L) * 180.0L - 90.0L;
            return (double)lat;
        }
    };


    struct BolusByteArray {
        uint8_t mArray[sizeof(Protocol::Bolus)];
        inline operator uint8_t*() {return mArray; }
    };

    struct CollarByteArray {
        uint8_t mArray[sizeof(Protocol::Collar)];
        inline operator uint8_t*() {return mArray; }
    };


    Protocol();
};

#endif // PROTOCOL_H
