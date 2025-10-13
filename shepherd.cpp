#include "shepherd.h"
#include "hardware/tools.h"

Shepherd::Shepherd(float step, float radius, QObject *parent)
    : QObject{parent}
{
    mRadius = radius;
    mStep = step;

    for( int i = 0; i < SHEPHERD_COEF_COUNT; i ++) {
        mK[i] = Tools::rnd(-10, 10);
    }

}

QPointF Shepherd::step()
{
    float x = sinf(mK[0]*mPos)*cosf(mK[1]*mPos) + cosf(mK[2]*mPos)*cosf(mK[3]*mPos)*sin(mK[4]*mPos);
    float y = sinf(mK[5]*mPos)*sinf(mK[6]*mPos) + sinf(mK[7]*mPos)*cosf(mK[8]*mPos)*sin(mK[9]*mPos);

    mPos += mStep;

    mLastPos = QPointF( x * mRadius, y * mRadius );

    return mLastPos;
}


void Shepherd::reset()
{
    mPos = 0;
}
