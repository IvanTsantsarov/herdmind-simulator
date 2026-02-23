#include <QRandomGenerator>
#include <QJsonDocument>
#include <QJsonObject>
#include "gateway.h"
#include "defines_settings.h"

#ifdef SIMULATION


Gateway::Gateway(const QSettings &settings)
{
    mChirpIP = settings.value(CHIRPSTACK_SECTION"/ip").toString();
    mChirpPort = settings.value(CHIRPSTACK_SECTION"/port").toUInt();
}

void Gateway::onUpdate()
{
    process();
}

void Gateway::onSend()
{

}

void Gateway::start()
{
    mSocket.bind(QHostAddress::AnyIPv4, mChirpPort);

    connect(&mSocket, &QUdpSocket::readyRead,
            this, &Gateway::onUdpReadyRead);

    connect(&mPullTimer, &QTimer::timeout,
            this, &Gateway::sendPullData);

    mPullTimer.start(5000);

    sendPullData();
}

void Gateway::sendPullData()
{
    QByteArray packet;

    packet.append((char)0x02);  // version

    quint16 token = QRandomGenerator::global()->generate();
    packet.append((char)(token >> 8));
    packet.append((char)(token & 0xFF));

    packet.append((char)0x02);  // PULL_DATA

    QByteArray gatewayEUI(8, 0x01);
    packet.append(gatewayEUI);

    mSocket.writeDatagram(packet,
                          QHostAddress(mChirpIP),
                          mChirpPort);
}


void Gateway::onUdpReadyRead()
{
    while (mSocket.hasPendingDatagrams()) {

        QByteArray datagram;
        datagram.resize(mSocket.pendingDatagramSize());
        mSocket.readDatagram(datagram.data(), datagram.size());

        quint8 identifier = datagram[3];

        if (identifier == 0x03) { // PULL_RESP

            QByteArray ba = datagram.mid(4);
            QJsonObject jobj = QJsonDocument::fromJson(ba).object();
            QJsonObject txpk = jobj["txpk"].toObject();

            QByteArray phy =
                QByteArray::fromBase64(txpk["data"].toString().toLatin1());

            // Deliver to device
            emit downlinkReceived(phy);
        }
    }
}


bool Gateway::sendToChirpStack(const QByteArray& phyPayload)
{
    QByteArray json = R"({
        "rxpk": [{
            "freq": 868.1,
            "datr": "SF7BW125",
            "codr": "4/5",
            "rssi": -30,
            "lsnr": 5.5,
            "data": ")";

    json += phyPayload.toBase64();
    json += R"("
        }]
    })";

    QByteArray packet;
    packet.reserve(5);

    // Semtech header
    packet.append((char)0x02); // version
    packet.append((char)0x00);
    packet.append((char)0x00);
    packet.append((char)0x00); // token
    packet.append((char)0x00); // PUSH_DATA

    QByteArray gatewayEUI(8, 0x01);
    packet.append(gatewayEUI);

    packet.append(json);

    qint64 sentSize = mSocket.writeDatagram(packet, QHostAddress(mChirpIP),mChirpPort);

    mIsSending = true;
    QTimer::singleShot(GATEWAY_SEND_INTERVAL, this, &Gateway::onStopSending);

    return sentSize == packet.size();
}

void Gateway::onStopSending()
{
    mIsSending = false;
}

#else


#endif


void Gateway::process()
{

}
