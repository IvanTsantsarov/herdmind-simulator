#pragma once

#include <QObject>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>

class BluetoothScanner : public QObject
{
    Q_OBJECT

    void initAndStartAgent();
    QBluetoothDeviceDiscoveryAgent *m_discoveryAgent = nullptr;
    QHash<QString, QBluetoothDeviceInfo> mInfo;

private slots:
    void onDeviceDiscovered(const QBluetoothDeviceInfo &device);
    void onScanFinished();
    void onScanError(QBluetoothDeviceDiscoveryAgent::Error error);

public:
    enum struct Event {
        Ok = 0,
        PermissionDenied,
        PermissionGranted,
        NewDevice,
        ScanFinished,
        ScanError
    };

    explicit BluetoothScanner(QObject *parent = nullptr);
    void startScan();
    QBluetoothDeviceInfo deviceInfo(const QString& desc);

signals:
    void newEvent(Event e, const QString& desc = QString());
};



#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QBluetoothUuid>
#include <QHash>

class BleClient : public QObject
{
    Q_OBJECT
public:
    explicit BleClient(QObject *parent = nullptr);
    ~BleClient() override;

    // Connect to a device discovered by QBluetoothDeviceDiscoveryAgent
    void connectToDevice(const QBluetoothDeviceInfo &device);
    void disconnectFromDevice();

    // Read / Write / Notify operations by Service UUID + Characteristic UUID
    void readCharacteristic(const QBluetoothUuid &serviceUuid, const QBluetoothUuid &charUuid);
    void writeCharacteristic(const QBluetoothUuid &serviceUuid, const QBluetoothUuid &charUuid,
                             const QByteArray &data, bool withResponse = true);
    void setNotificationsEnabled(const QBluetoothUuid &serviceUuid, const QBluetoothUuid &charUuid,
                                 bool enabled);

signals:
    void connected();
    void disconnected();
    void servicesReady();
    void characteristicRead(const QBluetoothUuid &charUuid, const QByteArray &value);
    void characteristicWritten(const QBluetoothUuid &charUuid, const QByteArray &value);
    void characteristicChanged(const QBluetoothUuid &charUuid, const QByteArray &newValue);
    void errorOccurred(const QString &message);

private slots:
    void onConnected();
    void onDisconnected();
    void onServiceDiscovered(const QBluetoothUuid &serviceUuid);
    void onServiceScanDone();
    void onServiceStateChanged(QLowEnergyService::ServiceState newState);

private:
    QLowEnergyController *m_controller = nullptr;
    QHash<QBluetoothUuid, QLowEnergyService *> m_services;
    int m_pendingServices = 0;
};


