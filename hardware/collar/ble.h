#pragma once

#include "../arduino.h"

#include "defines.h"

class Collar;

class Ble {

public:
    enum struct Event
    {
        None = 0,
        Connect,
        Disconnect,
        Error,
    };

    struct EventContex {
        Event mEvent;
        String mOther;
        Collar* mCollar;
    };

    typedef void (*BleEventCallback)(EventContex* ctx);


private:
    int64_t mOnMsec = 0;
    String mBuffer;
    int mBufferLen = 0;
    int mBufferPos = 0;

    class ServerCallbacks : public NimBLEServerCallbacks {

        Collar* mCollar = nullptr;
        BleEventCallback mCallback = nullptr;

    public:
        explicit ServerCallbacks(Ble* owner);
        void setParams(BleEventCallback callback, Collar* collar);
        void onConnect(NimBLEServer*, NimBLEConnInfo& info) override;
        void onDisconnect(NimBLEServer*, NimBLEConnInfo& info, int) override;

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

    ServerCallbacks mServerCallbacks{this};
    RxCallbacks mRxCallbacks{this};

    uint16_t mConnectionHandle = 0;
    String mIncoming;

    volatile bool mIsOn = false;
    volatile bool mIsConnected = false;
    volatile bool mIsReceived = false;

public:

    Ble() {}
    ~Ble() {off();}

    void on(const char* deviceName, BleEventCallback callback, Collar* collar);
    void off();
    inline bool isOn(){ return mIsOn; }
    bool update();

    bool available(){ return mBuffer.length() > mBufferPos; }
    char read();

    int64_t onMsec();

    // The phone must subscribe to TX notifications to receive this.
    void send(const String& message);

    bool isConnected() const {
        return mIsConnected;
    }
};

