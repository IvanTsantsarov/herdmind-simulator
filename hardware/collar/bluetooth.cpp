#include "bluetooth.h"

void Ble::on(const char *deviceName) {
    if (mIsOn) return;

    NimBLEDevice::init(deviceName);

    mServer = NimBLEDevice::createServer();
    mServer->setCallbacks(new ServerCallbacks(this));

    NimBLEService* service = mServer->createService(BLE_SE_UUID);

    mTxCharacteristic = service->createCharacteristic(
        BLE_TX_UUID, NIMBLE_PROPERTY::NOTIFY);

    mRxCharacteristic = service->createCharacteristic(
        BLE_RX_UUID, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
    mRxCharacteristic->setCallbacks(new RxCallbacks(this));

    service->start();

    NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();
    advertising->addServiceUUID(BLE_SE_UUID);
    advertising->start();

    mIsOn = true;
    DBG("[Ble]Advertising started");
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

    DBG("[Ble]Stopped");
}

String Ble::read()
{
    mIsReceived = false;
    String message = mIncoming;
    mIncoming = "";
    return message;
}

void Ble::send(const String &message)
{
    if (!mIsOn || !mIsConnected || mTxCharacteristic == nullptr) return;

    mTxCharacteristic->setValue(message.c_str());
    mTxCharacteristic->notify();
}

void Ble::ServerCallbacks::onConnect(NimBLEServer *, NimBLEConnInfo &info)
{
    mOwner->mConnectionHandle = info.getConnHandle();
    mOwner->mIsConnected = true;
    DBG("[Ble]Client connected");
}

void Ble::ServerCallbacks::onDisconnect(NimBLEServer *, NimBLEConnInfo &, int) {
    mOwner->mIsConnected = false;
    DBG("[Ble]Client disconnected");
}

void Ble::RxCallbacks::onWrite(NimBLECharacteristic *characteristic, NimBLEConnInfo &)
{
    std::string value = characteristic->getValue();
    mOwner->mIncoming = String(value.c_str());
    mOwner->mIsReceived = true;
}
