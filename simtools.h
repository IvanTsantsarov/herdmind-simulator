#ifndef SIMTOOLS_H
#define SIMTOOLS_H

#include <QVector>
#include <QVector2D>
#include <QSettings>
#include "hardware/collar/collar.h"

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
    QByteArray mJoinEUI;

public:
    SimTools(const QSettings &settings);

    inline QByteArray joinEUI(){ return mJoinEUI; }

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
    static QByteArray genHex(int count);
    static QByteArray genAesKey(){ return genHex(16); }

    bool sendToChirpStack(const QByteArray& phyPayload);

    bool sendCollar(const Collar& collar);
};

extern SimTools* gSimTools;

#endif // SIMTOOLS_H
