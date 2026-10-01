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
    // Enable battery voltage measurement
    digitalWrite(BAT_CTRL_PIN, HIGH);
    delay(10);

    const uint32_t adc_mV = analogReadMilliVolts(BAT_READ_PIN);

    // 390k / 100k divider => 4.9x
    mVoltage = adc_mV * 4.9f / 1000.0f;

    // A connected Li-ion battery should be several volts.
    mIsPresent = mVoltage >= 2.5f;

    const float percentage =
        (mVoltage - 3.2f) / (4.2f - 3.2f) * 100.0f;

    mPercentage = constrain(percentage, 0.0f, 100.0f);

    // Disable the measurement circuit when finished.
    digitalWrite(BAT_CTRL_PIN, LOW);
}