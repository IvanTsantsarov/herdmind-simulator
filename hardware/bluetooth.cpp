#include "bluetooth.h"

#include <QCoreApplication>
#include <QDebug>
#include <QPermissions>
#include <QBluetoothAddress>
#include <QBluetoothUuid>

BluetoothScanner::BluetoothScanner(QObject *parent)
    : QObject(parent)
{
}

void BluetoothScanner::startScan()
{
    QBluetoothPermission bluetoothPermission;
    bluetoothPermission.setCommunicationModes(QBluetoothPermission::Access);

    switch (qApp->checkPermission(bluetoothPermission)) {
    case Qt::PermissionStatus::Undetermined:
        qApp->requestPermission(bluetoothPermission, this, &BluetoothScanner::startScan);
        return;
    case Qt::PermissionStatus::Denied:
        qWarning() << "Bluetooth permission denied by the user.";
        QCoreApplication::quit();
        return;
    case Qt::PermissionStatus::Granted:
        initAndStartAgent();
        break;
    }
}

void BluetoothScanner::initAndStartAgent()
{
    if (!m_discoveryAgent) {
        m_discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
        m_discoveryAgent->setLowEnergyDiscoveryTimeout(10000); // 10 seconds

        connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
                this, &BluetoothScanner::onDeviceDiscovered);
        connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
                this, &BluetoothScanner::onScanFinished);
        connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred,
                this, &BluetoothScanner::onScanError);
    }

    qInfo() << "Starting Bluetooth scan...";
    // Use QBluetoothDeviceDiscoveryAgent::LowEnergyMethod to scan strictly for BLE
    m_discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::ClassicMethod |
                            QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

void BluetoothScanner::onDeviceDiscovered(const QBluetoothDeviceInfo &device)
{
    QString type = (device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration)
    ? "BLE" : "Classic";

    // On macOS/iOS, hardware MAC addresses are hidden for privacy; deviceUuid() is used instead
    QString identifier = device.address().isNull()
                             ? device.deviceUuid().toString()
                             : device.address().toString();

    qInfo() << QString("[%1] Name: %2 | ID: %3 | RSSI: %4 dBm")
                   .arg(type)
                   .arg(device.name().isEmpty() ? "Unknown" : device.name())
                   .arg(identifier)
                   .arg(device.rssi());
}

void BluetoothScanner::onScanFinished()
{
    qInfo() << "Bluetooth scan completed.";
    QCoreApplication::quit();
}

void BluetoothScanner::onScanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    qWarning() << "Bluetooth discovery error:" << error << "-" << m_discoveryAgent->errorString();
    QCoreApplication::quit();
}
