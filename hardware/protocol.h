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
public:
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

    inline static uint32_t encodeLat(double latDeg)
    {
        latDeg = std::clamp(latDeg, -90.0, 90.0);
        // map [-90, 90] -> [0, 2^32-1]
        long double u = ( (long double)(latDeg + 90.0L) / 180.0L ) * 4294967295.0L;
        return (uint32_t) llround(u);
    }

    inline static double decodeLat(uint32_t latInt) {
        long double lat = ((long double)latInt / 4294967295.0L) * 180.0L - 90.0L;
        return (double)lat;
    }

    inline static uint32_t encodeLon(double lonDeg)
    {
        lonDeg = std::clamp(lonDeg, -180.0, 180.0);
        // map [-180, 180] -> [0, 2^32-1]
        long double u = ( (long double)(lonDeg + 180.0L) / 360.0L ) * 4294967295.0L;
        return (uint32_t) llround(u);
    }

    inline static double decodeLon(uint32_t lonInt)
    {
        // map [0, 2^32-1] -> [-180, 180]
        long double lon = ((long double)lonInt / 4294967295.0L) * 360.0L - 180.0L;
        return (double)lon;
    }


    #pragma pack(push, 1)

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

            Package = 10,

            // First are individual messages
            Light = 16,
            Sound = 17,
            Shock = 18,

            // Mass messages starts from 128
            SetupFence = 128,
            FenceOn,
            FenceOff,
        };

        Event mEvent = Event::None;       // mEvent type level
        uint32_t mTimestamp = timestamp();  // unix timestamp
        uint32_t mLatitude = 0;   // positioning mLatitude
        uint32_t mLongitude = 0;  // positioning mLongitude
        uint8_t mRssi = 0;        // signal level
        uint8_t mBattery = 0;     // mBattery level

        inline void fromByteArray(const uint8_t* array) {
            int offset = 0;
            mEvent   = (Event)array[ offset++ ];
            mTimestamp = Protocol::readUint32(array, offset); offset += sizeof(uint32_t);
            mLatitude = Protocol::readUint32(array,  offset); offset += sizeof(uint32_t);
            mLongitude = Protocol::readUint32(array, offset); offset += sizeof(uint32_t);

            mRssi    = array[ offset++];
            mBattery = array[ offset++];
        }

        inline CollarByteArray toByteArray()  const {
            CollarByteArray ba;

            int offset = 0;
            ba[offset++]   = (uint8_t)mEvent;
            Protocol::writeUint32(mTimestamp, ba, offset); offset += sizeof(uint32_t);
            Protocol::writeUint32(mLatitude, ba, offset); offset += sizeof(uint32_t);
            Protocol::writeUint32(mLongitude,  ba, offset); offset += sizeof(uint32_t);

            ba[offset++] = mRssi;
            ba[offset++] = mBattery;
            return ba;
        }

        inline void encodeLat(double latDeg)
        {
            mLatitude = Protocol::encodeLat(latDeg);
        }

        inline double decodeLat() {
            return Protocol::decodeLat(mLatitude);
        }

        inline void encodeLon(double lonDeg)
        {
            mLongitude = Protocol::encodeLon(lonDeg);
        }

        inline double decodeLon()
        {
            return Protocol::decodeLon(mLongitude);
        }

    };

    #pragma pack(pop)

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
