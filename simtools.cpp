#include "simtools.h"
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
