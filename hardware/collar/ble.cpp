#include "ble.h"

void Ble::on(const char *deviceName, BleEventCallback callback, Collar* collar) {
    if (mIsOn) {
        return;
    }

    mOnMsec = millis();

    NimBLEDevice::init(deviceName);

    mServer = NimBLEDevice::createServer();
    mServer->setCallbacks(&mServerCallbacks);
    mServerCallbacks.setParams(callback, collar);

    NimBLEService* service = mServer->createService(BLE_SE_UUID);

    mTxCharacteristic = service->createCharacteristic(
        BLE_TX_UUID, NIMBLE_PROPERTY::NOTIFY);

    mRxCharacteristic = service->createCharacteristic(
        BLE_RX_UUID, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
    mRxCharacteristic->setCallbacks(&mRxCallbacks);

    service->start();

    NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();
    advertising->addServiceUUID(BLE_SE_UUID);
    advertising->setName(deviceName);
    advertising->enableScanResponse(true);
    advertising->start();

    mIsOn = true;
    DBG(String("[Ble]Advertising started as <") + deviceName + ">");

    mOnMsec = millis();
}

void Ble::off() {
    if (!mIsOn) {
        return;
    }

    if (mServer != nullptr && mIsConnected) {
        mServer->disconnect(mConnectionHandle);
    }

    NimBLEDevice::deinit(true);

    mServer = nullptr;
    mRxCharacteristic = nullptr;
    mTxCharacteristic = nullptr;
    mIsOn = false;
    mIsConnected = false;
    mIsReceived = false;
    mOnMsec = 0;

    DBG("[Ble]Stopped");
}

bool Ble::update()
{
    if( !isOn() || !mIsReceived ) {
        return false;
    }

    mBuffer += mIncoming;
    mBufferLen = mBuffer.length();
    mIsReceived = false;
    mIncoming = "";
    return true;
}

char Ble::read()
{
    if( !mBufferLen) {
        return 0;
    }

    if( mBufferPos >= mBufferLen ) {
        mBufferPos = 0;
        mBuffer = "";
        return 0;
    }

    return mBuffer[mBufferPos++];
}

int64_t Ble::onMsec()
{
    return millis() - mOnMsec;
}

void Ble::send(const String &message)
{
    if (!mIsOn || !mIsConnected || mTxCharacteristic == nullptr) return;

    mTxCharacteristic->setValue(message.c_str());
    mTxCharacteristic->notify();
}

Ble::ServerCallbacks::ServerCallbacks(Ble *owner)
    : mOwner(owner)
{

}

void Ble::ServerCallbacks::setParams(BleEventCallback callback, Collar *collar)
{
    mCollar = collar, mCallback = callback;
}

void Ble::ServerCallbacks::onConnect(NimBLEServer *, NimBLEConnInfo &info)
{
    mOwner->mConnectionHandle = info.getConnHandle();
    mOwner->mIsConnected = true;
    DBG("[Ble]Client connected");

    if( mCallback ) {
        EventContex ctx;
        ctx.mCollar = mCollar;
        ctx.mEvent = Event::Connect;
        // ctx.mOther = info.
        mCallback( &ctx );
    }
}

void Ble::ServerCallbacks::onDisconnect(NimBLEServer *, NimBLEConnInfo &info, int) {
    mOwner->mIsConnected = false;
    DBG("[Ble]Client disconnected");

    if( mCallback ) {
        EventContex ctx;
        ctx.mCollar = mCollar;
        ctx.mEvent = Event::Disconnect;
        // ctx.mOther = info.
        mCallback( &ctx );
    }

}

void Ble::RxCallbacks::onWrite(NimBLECharacteristic *characteristic, NimBLEConnInfo &)
{
    std::string value = characteristic->getValue();
    mOwner->mIncoming = String(value.c_str());
    mOwner->mIsReceived = true;
}
