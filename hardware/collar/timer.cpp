#include "timer.h"

#include <sys/time.h>

#define TIMER_MAX_STR 64

RTC_DATA_ATTR uint64_t Timer::mMsec = 1767225600000ULL;
RTC_DATA_ATTR uint32_t Timer::mSleepMsec = 0;


Timer::Timer() {
    mMsecStart = millis();
}

void Timer::setMsecEpoch(uint64_t msec)
{
    mMsec = msec;
    mMsecStart = millis();
}

void Timer::wakeup()
{
    mMsecStart = millis();
    mMsec += (uint64_t) mSleepMsec + mMsecStart;
}

tm *Timer::gmt()
{
    int64_t deltaMsec = millis() - mMsecStart;
    time_t t = (mMsec + deltaMsec) / 1000ULL;
    return gmtime(&t);
}

String Timer::format(const char *fmt)
{
    char str[TIMER_MAX_STR+1] = {0};
    strftime(str, TIMER_MAX_STR, fmt, gmt());
    return String(str);
}

