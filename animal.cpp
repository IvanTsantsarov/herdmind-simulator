#include "animal.h"
#include "bolus/bolus.h"
#include "collar/collar.h"

#define MIN_FLOAT 1e-6f

void Animal::interact(const QVector2D &p, float attractionPower, float attractionDistance, float repellingDistance)
{
    QVector2D f = QVector2D(p) - mPosition;
    float lensq = f.lengthSquared();

    if( lensq > attractionDistance*attractionDistance ) {
        mVelocity += f.normalized() * attractionPower;
        mRotationAngleGoal = qAtan2(mVelocity.y(), mVelocity.x());
    }else
    if( lensq < repellingDistance*repellingDistance ) {
        mVelocity -= f.normalized() * attractionPower;
        mRotationAngleGoal = qAtan2(mVelocity.y(), mVelocity.x());
    }
}

bool Animal::collide(Animal *a, float minDistance )
{
    float distance = (a->p() - p()).lengthSquared();

    // if both are too far - exit
    if( distance > (minDistance*minDistance)) {
        return false;
    }


    // if both not approaching to each other
    float distanceNext = ((a->p()+a->v()) - (p()+v())).lengthSquared();
    if( distanceNext > distance ) {
        return false;
    }

    QVector2D n = p() - a->p();
    QVector2D rv = v() - a->v();

    float len = n.length();
    if (len <= MIN_FLOAT) {
        float rvLen = rv.length();
        if (rvLen <= MIN_FLOAT) return false;
        n = rv / rvLen;
    } else {
        n /= len;
    }

    float vn = -QVector2D::dotProduct(rv, n);
    mVelocity += vn * n;
    a->mVelocity -= vn * n;

    mRotationAngleGoal = qAtan2(mVelocity.y(), mVelocity.x());

    return true;
}

void Animal::updateSpeed(float maxSpeed, float friction, float rotationFading)
{
    if( mVelocity.length() > maxSpeed) {
        mVelocity = mVelocity.normalized() * maxSpeed;
    }

    mVelocity *= (1.0f - friction);

    mRotationAngle = mRotationAngle + rotationFading * (mRotationAngleGoal - mRotationAngle);

    mPosition += mVelocity;
}



Animal::Animal(float x, float y, bool isBolus, bool isCollar)
{
    mPosition = QVector2D(x, y);
    mVelocity = QVector2D(0, 0);

    mBolus = new Bolus();
    mCollar = new Collar();
}

Animal::~Animal()
{
    if( mBolus ) {
        delete mBolus;
        mBolus = nullptr;
    }

    if( mCollar ) {
        delete mCollar;
        mCollar = nullptr;
    }
}
