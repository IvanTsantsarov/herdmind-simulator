#pragma once

#include <QObject>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>

class BluetoothScanner : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothScanner(QObject *parent = nullptr);
    void startScan();

private slots:
    void onDeviceDiscovered(const QBluetoothDeviceInfo &device);
    void onScanFinished();
    void onScanError(QBluetoothDeviceDiscoveryAgent::Error error);

private:
    void initAndStartAgent();
    QBluetoothDeviceDiscoveryAgent *m_discoveryAgent = nullptr;
};

