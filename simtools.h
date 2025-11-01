#ifndef SIMTOOLS_H
#define SIMTOOLS_H

#include <QVector>
#include <QVector2D>

class SimTools
{
public:
    SimTools();

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
