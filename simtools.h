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
    QByteArray mAppSKey, mNwkSKey;


    QByteArray buildFRMPayload(const QList<CollarData>& herd);

    QByteArray encryptFRMPayload(const QByteArray& payload,
                                 quint32 devAddr,
                                 quint32 fCnt);

    QByteArray aesCmac( const QByteArray& data );
    QByteArray calculateMIC(
        const QByteArray& msg,
        quint32 devAddr,
        quint32 fCnt);

    QByteArray aes128EncryptBlock( const QByteArray& block);

    QByteArray buildPHYPayload(QByteArray frmPayload,
                               quint32 devAddr,
                               quint32 fCnt);

    void sendToChirpStack(const QByteArray& phyPayload);

public:
    SimTools(QSettings& settings);

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
};

#endif // SIMTOOLS_H
