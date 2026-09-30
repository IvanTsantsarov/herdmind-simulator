#ifndef BATTERY_H
#define BATTERY_H

class Battery
{
    int mPercentage = 0;
    float mVoltage = 0.0f;
    bool mIsPresent = false;
public:
    Battery();
    void setup();
    void update();

    inline bool isPresent(){ return mIsPresent; }
    inline float voltage(){ return mVoltage; }
    inline int level(){ return mPercentage; }
};

#endif // BATTERY_H
