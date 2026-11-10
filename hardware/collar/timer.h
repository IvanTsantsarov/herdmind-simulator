#ifndef COLLAR_TIMER_H
#define COLLAR_TIMER_H

#include "../arduino.h"

class Timer
{
    static RTC_DATA_ATTR uint32_t mSleepMsec;
    static RTC_DATA_ATTR uint64_t mMsec;
    uint64_t mMsecStart = 0;


public:
    Timer();

    // Set milliseconds from Epoch (1970)
    void setMsecEpoch(uint64_t msec);
    void setEpoch(uint64_t secEpoch){ setMsecEpoch(secEpoch * 1000ULL); }

    inline uint64_t msec(){ return mMsec + millis() - mMsecStart; }
    inline uint64_t epoch(){ return msec() / 1000ULL; }

    void sleep(uint64_t msec);
    inline uint64_t sleepMsec(){ return mSleepMsec;}; // returns sleep duration

    struct tm* gmt();

    String format(const char* fmt);

    inline String timeStr(){ return format("%H:%M:%S"); }
    String dateStr(){ return format("%d.%b.%y"); }
    String dateTimeStr(){ return format("%H:%M:%S, %d.%b.%y"); }
};



#endif // COLLAR_TIMER_H
