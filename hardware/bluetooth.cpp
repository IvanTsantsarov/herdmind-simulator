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
    mInfo.clear();

    QBluetoothPermission bluetoothPermission;
    bluetoothPermission.setCommunicationModes(QBluetoothPermission::Access);

    switch (qApp->checkPermission(bluetoothPermission)) {
    case Qt::PermissionStatus::Undetermined:
        qApp->requestPermission(bluetoothPermission, this, &BluetoothScanner::startScan);
        return;
    case Qt::PermissionStatus::Denied:
        qWarning() << "Bluetooth permission denied by the user.";
        emit newEvent(Event::PermissionDenied);
        return;
    case Qt::PermissionStatus::Granted:
        emit newEvent(Event::PermissionGranted);
        initAndStartAgent();
        break;
    }
}

QBluetoothDeviceInfo BluetoothScanner::deviceInfo(const QString &desc)
{
    if( mInfo.contains(desc) ) {
        return mInfo[desc];
    }
    return QBluetoothDeviceInfo();
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

    QString desc = QString("%1|%2|%3").arg(device.name()).arg(identifier).arg(device.rssi());
    mInfo[desc] = device;
    emit newEvent(Event::NewDevice, desc);
}

void BluetoothScanner::onScanFinished()
{
    qInfo() << "Bluetooth scan completed.";
    emit newEvent(Event::ScanFinished);
}

void BluetoothScanner::onScanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    emit newEvent(Event::ScanError, m_discoveryAgent->errorString());
    qWarning() << "Bluetooth discovery error:" << error << "-" << m_discoveryAgent->errorString();
}



BleClient::BleClient(QObject *parent)
    : QObject(parent)
{
}

BleClient::~BleClient()
{
    disconnectFromDevice();
}

void BleClient::connectToDevice(const QBluetoothDeviceInfo &device)
{
    disconnectFromDevice();

    m_controller = QLowEnergyController::createCentral(device, this);

    connect(m_controller, &QLowEnergyController::connected,
            this, &BleClient::onConnected);
    connect(m_controller, &QLowEnergyController::disconnected,
            this, &BleClient::onDisconnected);
    connect(m_controller, &QLowEnergyController::serviceDiscovered,
            this, &BleClient::onServiceDiscovered);
    connect(m_controller, &QLowEnergyController::discoveryFinished,
            this, &BleClient::onServiceScanDone);
    connect(m_controller, &QLowEnergyController::errorOccurred,
            this, [this](QLowEnergyController::Error error) {
                Q_UNUSED(error);
                emit errorOccurred(m_controller->errorString());
            });

    qInfo() << "Connecting to" << device.name() << "...";
    m_controller->connectToDevice();
}

void BleClient::disconnectFromDevice()
{
    qDeleteAll(m_services);
    m_services.clear();
    m_pendingServices = 0;

    if (m_controller) {
        if (m_controller->state() != QLowEnergyController::UnconnectedState)
            m_controller->disconnectFromDevice();
        m_controller->deleteLater();
        m_controller = nullptr;
    }
}

void BleClient::onConnected()
{
    qInfo() << "Connected! Discovering GATT services...";
    emit connected();
    m_controller->discoverServices();
}

void BleClient::onDisconnected()
{
    qInfo() << "Disconnected from device.";
    emit disconnected();
}

void BleClient::onServiceDiscovered(const QBluetoothUuid &serviceUuid)
{
    QLowEnergyService *service = m_controller->createServiceObject(serviceUuid, this);
    if (!service) {
        qWarning() << "Could not create service object for" << serviceUuid.toString();
        return;
    }

    m_services.insert(serviceUuid, service);
    m_pendingServices++;

    connect(service, &QLowEnergyService::stateChanged,
            this, &BleClient::onServiceStateChanged);
    connect(service, &QLowEnergyService::characteristicRead,
            this, [this](const QLowEnergyCharacteristic &c, const QByteArray &val) {
                emit characteristicRead(c.uuid(), val);
            });
    connect(service, &QLowEnergyService::characteristicWritten,
            this, [this](const QLowEnergyCharacteristic &c, const QByteArray &val) {
                emit characteristicWritten(c.uuid(), val);
            });
    connect(service, &QLowEnergyService::characteristicChanged,
            this, [this](const QLowEnergyCharacteristic &c, const QByteArray &val) {
                emit characteristicChanged(c.uuid(), val);
            });
    connect(service, &QLowEnergyService::errorOccurred,
            this, [this](QLowEnergyService::ServiceError err) {
                emit errorOccurred(QString("GATT Service Error: %1").arg(err));
            });

    // Discover all characteristics & descriptors inside this service
    service->discoverDetails();
}

void BleClient::onServiceScanDone()
{
    qInfo() << "Service list discovered. Waiting for characteristic details...";
    if (m_pendingServices == 0)
        emit servicesReady();
}

void BleClient::onServiceStateChanged(QLowEnergyService::ServiceState newState)
{
    if (newState == QLowEnergyService::RemoteServiceDiscovered) {
        m_pendingServices--;
        if (m_pendingServices <= 0) {
            m_pendingServices = 0;
            qInfo() << "All GATT services and characteristics ready!";
            emit servicesReady();
        }
    }
}

void BleClient::readCharacteristic(const QBluetoothUuid &serviceUuid, const QBluetoothUuid &charUuid)
{
    QLowEnergyService *service = m_services.value(serviceUuid, nullptr);
    if (!service) {
        emit errorOccurred("Service not found: " + serviceUuid.toString());
        return;
    }

    const QLowEnergyCharacteristic characteristic = service->characteristic(charUuid);
    if (!characteristic.isValid()) {
        emit errorOccurred("Characteristic not found: " + charUuid.toString());
        return;
    }

    service->readCharacteristic(characteristic);
}

void BleClient::writeCharacteristic(const QBluetoothUuid &serviceUuid, const QBluetoothUuid &charUuid,
                                    const QByteArray &data, bool withResponse)
{
    QLowEnergyService *service = m_services.value(serviceUuid, nullptr);
    if (!service) {
        emit errorOccurred("Service not found: " + serviceUuid.toString());
        return;
    }

    const QLowEnergyCharacteristic characteristic = service->characteristic(charUuid);
    if (!characteristic.isValid()) {
        emit errorOccurred("Characteristic not found: " + charUuid.toString());
        return;
    }

    auto mode = withResponse ? QLowEnergyService::WriteWithResponse
                             : QLowEnergyService::WriteWithoutResponse;
    service->writeCharacteristic(characteristic, data, mode);
}

void BleClient::setNotificationsEnabled(const QBluetoothUuid &serviceUuid, const QBluetoothUuid &charUuid,
                                        bool enabled)
{
    QLowEnergyService *service = m_services.value(serviceUuid, nullptr);
    if (!service)
        return;

    const QLowEnergyCharacteristic characteristic = service->characteristic(charUuid);
    if (!characteristic.isValid())
        return;

    // Standard Client Characteristic Configuration Descriptor (CCCD - 0x2902)
    const QLowEnergyDescriptor cccd = characteristic.descriptor(
        QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);

    if (cccd.isValid()) {
        service->writeDescriptor(cccd, enabled ? QByteArray::fromHex("0100")
                                 : QByteArray::fromHex("0000"));
    }
}