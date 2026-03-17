#include <openssl/aes.h>
#include <openssl/cmac.h>
#include <openssl/evp.h>
#include <openssl/core_names.h>
#include <QCryptographicHash>
#include <QIODevice>
#include <QRandomGenerator>
#include <QUdpSocket>
#include <QFile>

#include "simtools.h"
#include "defines_settings.h"
#include "hardware/tools.h"

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
    return QRandomGenerator::global()->bounded(minVal, maxVal);
}

QByteArray SimTools::genHex(int bytesCount)
{
    QByteArray result;
    int charsCount = bytesCount*2;
    result.reserve(charsCount);

    for( auto i = 0; i < charsCount; i ++) {
        int v = gen(0, 15 );
        int ch = v < 10 ? v + '0' : v + 'A' - 10;
        result.append(ch);
    }

    return result;
}

QByteArray SimTools::fileRead(const QString &path, bool *isOk)
{
    QFile file( path );
    if ( file.open(QIODevice::ReadOnly) )
    {
        if( isOk ) {
            *isOk = true;
        }
        return file.readAll();
    }

    if( isOk ) {
        *isOk = false;
    }

    return QByteArray();
}

bool SimTools::fileExists(const QString &path)
{
    bool isFileExisting = QFile::exists(path);
    return isFileExisting;
}

bool SimTools::fileWrite(const QString &path, const QByteArray &content, bool isOverwrite)
{
    if( !isOverwrite && QFile::exists(path) ) {
        return true;
    }

    QFile file( path );
    if ( file.open(QIODevice::WriteOnly) )
    {
        QTextStream stream( &file );
        stream << content;
        return true;
    }

    return false;
}

bool SimTools::fileCompare(const QString &pathFile1, const QString &pathFile2, bool* isOk )
{
    QFile f1(pathFile1);
    QFile f2(pathFile2);

    if(isOk) *isOk = true;

    if( f1.size() != f2.size() ) {
        return false;
    }

    QByteArray data1 = SimTools::fileRead(pathFile1, isOk);
    if( isOk && !*isOk) return false;

    QByteArray data2 = SimTools::fileRead(pathFile2, isOk);
    if( isOk && !*isOk) return false;

    const QByteArray md51 = QCryptographicHash::hash(data1, QCryptographicHash::Md5);
    const QByteArray md52 = QCryptographicHash::hash(data2, QCryptographicHash::Md5);

    return md51 == md52;
}

bool SimTools::fileRestoreResources(const QString &fileName)
{
    QString srcPath = "://" + fileName;
    bool isOverwrite = false;
    bool isExists = false;

    if( !QFile::exists(fileName) ) {
        isOverwrite = true;
    }else {
        isExists = true;
        bool isOk = false;
        isOverwrite = !SimTools::fileCompare(srcPath, fileName, &isOk);
        if( !isOk ) {
            qCritical() << "Files" << srcPath << fileName << "error";
            return false;
        }
    }

    if( !isOverwrite ) {
        return true;
    }

    QFile f(srcPath);

    if( isExists ) {
        if( !QFile::remove(fileName)) {
            qCritical() << "Error restoring res file - destination file not writable:" << fileName;
            return false;
        }
    }

    if( !f.copy(fileName) ) {
        qCritical() << "Error restoring res file" << fileName << f.errorString();
        return false;
    }

    qInfo() << "File" << fileName << " restored from resources.";

    return true;
}




SimTools::SimTools(const QSettings &settings)
{
    mAppId  = settings.value(CHIRPSTACK_SECTION"/appId").toByteArray();

    mBolusProfileId  = settings.value(CHIRPSTACK_SECTION"/bolusProfileId").toByteArray();
    mCollarProfileId  = settings.value(CHIRPSTACK_SECTION"/collarProfileId").toByteArray();
    mGatewayProfileId  = settings.value(CHIRPSTACK_SECTION"/gatewayProfileId").toByteArray();


}

QString SimTools::profileId(LoraDev::Profile profile) {

    switch( profile )
    {
    case LoraDev::Profile::None :
        Q_ASSERT(0);
        break;
    case LoraDev::Profile::Bolus: return mBolusProfileId;
    case LoraDev::Profile::Collar: return mCollarProfileId;
    }

    return "";
}


