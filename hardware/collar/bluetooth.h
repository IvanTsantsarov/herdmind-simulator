#pragma once //COLLAR_BLUETOOTH_H

#include "../arduino.h"

#include "defines.h"

class Ble {
public:

    Ble() {}
    ~Ble() {off();}

    void on(const char* deviceName = "MyHeltecDevice");
    void off();
    bool available() const { return mIsReceived; }
    String read();

    // The phone must subscribe to TX notifications to receive this.
    void send(const String& message);

    bool isConnected() const {
        return mIsConnected;
    }

private:
    class ServerCallbacks : public NimBLEServerCallbacks {
    public:
        explicit ServerCallbacks(Ble* owner) : mOwner(owner) {}
        void onConnect(NimBLEServer*, NimBLEConnInfo& info) override;
        void onDisconnect(NimBLEServer*, NimBLEConnInfo&, int) override;

    private:
        Ble* mOwner;
    };

    class RxCallbacks : public NimBLECharacteristicCallbacks {
    public:
        explicit RxCallbacks(Ble* owner) : mOwner(owner) {}
        void onWrite(NimBLECharacteristic* characteristic, NimBLEConnInfo&) override;

    private:
        Ble* mOwner;
    };

    NimBLEServer* mServer = nullptr;
    NimBLECharacteristic* mRxCharacteristic = nullptr;
    NimBLECharacteristic* mTxCharacteristic = nullptr;

    uint16_t mConnectionHandle = 0;
    String mIncoming;

    volatile bool mIsOn = false;
    volatile bool mIsConnected = false;
    volatile bool mIsReceived = false;
};


////
/// \brief ble
///// Example
/*
Ble ble;

void setup() {
    Serial.begin(115200);
    ble.on();
}

void loop() {
    if (ble.available()) {
        String message = ble.read();
        Serial.println("Received: " + message);
        ble.send("Reply: " + message);
    }

    // Call ble.off() when Ble is no longer needed.
    delay(10);
}
*/