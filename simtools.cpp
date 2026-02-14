#include <openssl/aes.h>
#include <openssl/cmac.h>
#include <openssl/evp.h>
#include <openssl/core_names.h>
#include <QIODevice>
#include <QUdpSocket>

#include "simtools.h"
#include "hardware/tools.h"

#define LORAWAN_SECTION "LoraWAN"

SimTools::HarmonicsGenerator::HarmonicsGenerator(float radius, int count,
                                                 float ampMin, float ampMax,
                                                 float wavelenMin, float wavelenMax)
{
    mCenters.reserve(count);
    mAmplitudes.reserve(count);
    mLength.reserve(count);

    for( auto i = 0; i < count; i++) {
        mAmplitudes.append( Tools::rnd(ampMin, ampMax) );
        mLength.append( Tools::rnd(wavelenMin, wavelenMax) );
        mCenters.append( QVector2D(Tools::rnd(-radius, radius), Tools::rnd(-radius, radius)) );
    }
}

float SimTools::HarmonicsGenerator::sum(float x, float y, float angle) {
    float result = 0.0f;
    QVector2D o(x, y);
    for( auto i = 0; i < mAmplitudes.count(); i ++ ) {
        QVector2D d = mCenters[i] - o;
        float distance = d.length();
        float len = mLength[i];
        float len_inv = 1.0f/len;
        int waves = distance * len_inv;
        float phase = (distance - waves * len) * len_inv * 2.0f * M_PI;
        result += mAmplitudes[i] * sinf( phase + angle );
    }
    return result;
}


QVector2D SimTools::rotated(const QVector2D &v, float deg)
{
    float rad = qDegreesToRadians(deg);
    float sina = sinf(rad);
    float cosa = cosf(rad);
    return QVector2D( v.x()*cosa - v.y() * sina, v.x()*sina + v.y()*cosa  );
}

float SimTools::clamped(float val, float min, float max)
{
    if( val > max ) return max;
    if( val < min ) return min;
    return val;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Lorawan simulation
////////////////////////////////////////////////////////////////////////////////////////////////////
///
QByteArray SimTools::buildFRMPayload(const QList<CollarData>& herd)
{
    QByteArray payload;
    QDataStream stream(&payload, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    stream << (quint8)herd.size();

    for (const auto& collar : herd) {

        stream.writeRawData((char*)&collar.mCollar,
                            sizeof(Collar::PackageOut));

        for (const auto& bolus : collar.mBoluses) {
            stream.writeRawData((char*)&bolus,
                                sizeof(Collar::PackageBolusOut));
        }
    }

    return payload;
}


QByteArray SimTools::aes128EncryptBlock( const QByteArray& block )
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    QByteArray out(16, 0);

    int outLen = 0;

    EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), nullptr,
                       reinterpret_cast<const unsigned char*>(mAppSKey.data()),
                       nullptr);

    EVP_CIPHER_CTX_set_padding(ctx, 0);

    EVP_EncryptUpdate(ctx,
                      reinterpret_cast<unsigned char*>(out.data()), &outLen,
                      reinterpret_cast<const unsigned char*>(block.data()), 16);

    EVP_CIPHER_CTX_free(ctx);

    return out;
}

QByteArray SimTools::encryptFRMPayload(const QByteArray& payload,
                             quint32 devAddr,
                             quint32 fCnt)
{
    QByteArray encrypted = payload;
    int blocks = (payload.size() + 15) / 16;

    for (int i = 0; i < blocks; i++) {

        QByteArray Ai(16, 0x00);

        Ai[0] = 0x01;         // encryption flags
        Ai[5] = 0x00;         // Dir = uplink

        memcpy(Ai.data() + 6, &devAddr, 4);
        memcpy(Ai.data() + 10, &fCnt, 4);

        Ai[15] = i + 1;

        QByteArray Si = aes128EncryptBlock(Ai);

        for (int j = 0; j < 16; j++) {
            int index = i * 16 + j;
            if (index < encrypted.size())
                encrypted[index] ^= Si[j];
        }
    }

    return encrypted;
}


QByteArray SimTools::aesCmac( const QByteArray& data)
{
    EVP_MAC* mac = EVP_MAC_fetch(nullptr, "CMAC", nullptr);
    EVP_MAC_CTX* ctx = EVP_MAC_CTX_new(mac);

    OSSL_PARAM params[] = {
        OSSL_PARAM_construct_utf8_string(
            OSSL_MAC_PARAM_CIPHER,
            const_cast<char*>("AES-128-CBC"), 0),
        OSSL_PARAM_END
    };

    EVP_MAC_init(ctx,
                 reinterpret_cast<const unsigned char*>(mNwkSKey.data()),
                 mNwkSKey.size(),
                 params);

    EVP_MAC_update(ctx,
                   reinterpret_cast<const unsigned char*>(data.data()),
                   data.size());

    unsigned char out[16];
    size_t outLen = 0;

    EVP_MAC_final(ctx, out, &outLen, sizeof(out));

    EVP_MAC_CTX_free(ctx);
    EVP_MAC_free(mac);

    return QByteArray(reinterpret_cast<char*>(out), 16);
}

QByteArray SimTools::calculateMIC(
                        const QByteArray& msg,
                        quint32 devAddr,
                        quint32 fCnt)
{
    QByteArray B0(16, 0x00);

    B0[0] = 0x49;
    B0[5] = 0x00; // uplink

    memcpy(B0.data() + 6, &devAddr, 4);
    memcpy(B0.data() + 10, &fCnt, 4);

    B0[15] = msg.size();

    QByteArray cmacInput = B0 + msg;

    QByteArray fullCmac = aesCmac(cmacInput);

    return fullCmac.left(4);
}


QByteArray SimTools::buildPHYPayload(QByteArray frmPayload,
                           quint32 devAddr,
                           quint32 fCnt)
{
    QByteArray phy;

    quint8 MHDR = 0x40; // Unconfirmed Data Up
    phy.append(MHDR);

    // FHDR
    phy.append((char*)&devAddr, 4);

    quint8 fCtrl = 0x00;
    phy.append(fCtrl);

    quint16 fCnt16 = fCnt & 0xFFFF;
    phy.append((char*)&fCnt16, 2);

    quint8 fPort = 1;
    phy.append(fPort);

    QByteArray encrypted = encryptFRMPayload(frmPayload,  devAddr, fCnt);

    phy.append(encrypted);

    QByteArray mic = calculateMIC( phy, devAddr, fCnt);

    phy.append(mic);

    return phy;
}


void SimTools::sendToChirpStack(const QByteArray& phyPayload)
{
    QUdpSocket socket;

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

    socket.writeDatagram(packet,
                         QHostAddress(mChirpIP),
                         mChirpPort);
}

SimTools::SimTools(QSettings &settings)
{
    mChirpIP = settings.value(LORAWAN_SECTION"/chirpIp").toString();
    mChirpPort = settings.value(LORAWAN_SECTION"/chirpPort").toUInt();
    mAppSKey = settings.value(LORAWAN_SECTION"/appSKey").toByteArray();
    mNwkSKey = settings.value(LORAWAN_SECTION"/nwkSKey").toByteArray();
}
