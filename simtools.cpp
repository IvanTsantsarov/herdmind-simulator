#include <openssl/aes.h>
#include <openssl/cmac.h>
#include <openssl/evp.h>
#include <openssl/core_names.h>
#include <QIODevice>
#include <QUdpSocket>

#include "simtools.h"
#include "hardware/tools.h"

#define CHIRPSTACK_SECTION "Chirpstack"

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

QByteArray SimTools::encryptAES( const QByteArray& block, const QByteArray& key )
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    QByteArray out(16, 0);

    int outLen = 0;

    EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), nullptr,
                       reinterpret_cast<const unsigned char*>(key.data()),
                       nullptr);

    EVP_CIPHER_CTX_set_padding(ctx, 0);

    EVP_EncryptUpdate(ctx,
                      reinterpret_cast<unsigned char*>(out.data()), &outLen,
                      reinterpret_cast<const unsigned char*>(block.data()), 16);

    EVP_CIPHER_CTX_free(ctx);

    return out;
}


QByteArray SimTools::aesCmac( const QByteArray& data, const QByteArray& key )
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
                 reinterpret_cast<const unsigned char*>(key.data()),
                 key.size(),
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

int SimTools::gen(int minVal, int maxVal)
{
    float rnd = ((float)rand() / (float)RAND_MAX);
    return round(minVal + ( maxVal - minVal ) * rnd);
}

QByteArray SimTools::genHex(int count)
{
    QByteArray result;
    result.reserve(count);

    for( auto i = 0; i < count; i ++) {
        int v = gen(0, 15 );
        int ch = v < 10 ? v + '0' : v + 'A' - 10;
        result.append(ch);
    }

    return result;
}



bool SimTools::sendToChirpStack(const QByteArray& phyPayload)
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

    qint64 sentSize = socket.writeDatagram(packet, QHostAddress(mChirpIP),mChirpPort);

    return sentSize == packet.size();
}

SimTools::SimTools(QSettings &settings)
{
    mChirpIP = settings.value(CHIRPSTACK_SECTION"/ip").toString();
    mChirpPort = settings.value(CHIRPSTACK_SECTION"/port").toUInt();
}
