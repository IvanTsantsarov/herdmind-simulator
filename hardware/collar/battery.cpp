#include "battery.h"

#include "../arduino.h"

#define BAT_READ_PIN 1      // GPIO1 / ADC1_CH0
#define BAT_CTRL_PIN 37     // GPIO37 / ADC_Ctrl

Battery::Battery()
{
}

void Battery::setup()
{
    // Enable the battery voltage divider.
    // IMPORTANT: On V4 this is ACTIVE HIGH.
    pinMode(BAT_CTRL_PIN, OUTPUT);
    digitalWrite(BAT_CTRL_PIN, HIGH);

    pinMode(BAT_READ_PIN, INPUT);

    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
}

void Battery::update()
{
    // GPIO37 must be HIGH before reading GPIO1.
    digitalWrite(BAT_CTRL_PIN, HIGH);

    // Give the divider/ADC a little time to settle.
    delay(10);

    const int rawAdc = analogRead(BAT_READ_PIN);

    // 390k / 100k divider:
    //
    // Vadc = Vbat * 100 / (390 + 100)
    // Vbat = Vadc * 4.9
    //
    mVoltage = (rawAdc / 4095.0f) * 3.3f * 4.9f;

    mIsPresent = (mVoltage > 1.0f && mVoltage < 4.35f);

    const float batteryPercentage =
        ((mVoltage - 3.2f) / (4.2f - 3.2f)) * 100.0f;

    mPercentage = constrain(batteryPercentage, 0.0f, 100.0f);
}