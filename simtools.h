#ifndef SIMTOOLS_H
#define SIMTOOLS_H

#include <QVector>
#include <QVector2D>
#include <QSettings>
#include "hardware/collar/collar.h"
#include "hardware/loradev.h"

#define AES_BYTES_LEN 16

class SimTools
{
public:

private:

    //////////////////////////////////////////
    /// Lorawan simulation
    //////////////////////////////////////////

    // Read them from settins file
    QString mChirpIP;
    quint16 mChirpPort;
    QByteArray mAppId; // generated in chirpstack after creating new application, should be in settings
    QByteArray mBolusProfileId, mCollarProfileId, mGatewayProfileId;

public:
    SimTools(const QSettings &settings);

    inline QByteArray appId(){ return mAppId; }
    QByteArray profileId(LoraDev::Profile profile);


    struct HarmonicsGenerator {

        float mStep  = 0.0f;

        QVector<float> mAmplitudes;
        QVector<float> mLength;
        QVector<QVector2D> mCenters;

        HarmonicsGenerator(float radius,
                           int count = 10,
                           float ampMin = 1.0f,
                           float ampMax = 1.0f,
                           float wavelenMin = 0.1f,
                           float wavelenMax = 10.0f
                           );
        float sum(float x, float y, float angle = 0.0f);
    };

    static QVector2D rotated( const QVector2D& v, float deg);

    static float clamped(float val, float min = 0.0f, float max = 1.0f);

    static QByteArray encryptAES(const QByteArray& block, const QByteArray &key);
    static QByteArray aesCmac(const QByteArray& data , const QByteArray &key);

    static int gen(int minVal, int maxVal);
    inline static int gen(int maxVal) { return gen(0, maxVal); }
    static QByteArray genHex(int count);
    static QByteArray genAesKey(){ return genHex(AES_BYTES_LEN); }

    bool fileWrite(const QString& path, const QByteArray& content, bool isOverwrite = true);

    bool sendToChirpStack(const QByteArray& phyPayload);
    bool sendCollar(const Collar& collar);
};

extern SimTools* gSimTools;

#endif // SIMTOOLS_H
