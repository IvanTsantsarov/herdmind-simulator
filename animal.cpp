#include "animal.h"
#include "hardware/bolus/bolus.h"
#include "hardware/collar/collar.h"

#define MIN_FLOAT 1e-6f

void Animal::updateDirection()
{
    mDirection = mVelocity.normalized();
    mRotationAngle = qAtan2(mDirection.y(), mDirection.x());

    if (mRotationAngle < 0)
        mRotationAngle += static_cast<float>(M_PI * 2); // make it in range 0..pi

}

void Animal::onThisBolusSent(void *package)
{
    foreach( Animal* animal, mObservers) {
        animal->onOtherBolusData(package, this);
    }
}

void Animal::onThisCollarSent(void *package)
{

}

void Animal::onOtherBolusData(void *package, Animal* from)
{
    from->mReadings ++;
}

float Animal::distanceSq(Animal *other)
{
    float dx = mPosition.x() - other->mPosition.x();
    float dy = mPosition.y() - other->mPosition.y();
    return dx*dx + dy*dy;
}

void Animal::react(const QVector2D &p, float attractionPower, float attractionDistance, float repellingDistance)
{
    QVector2D f = QVector2D(p) - mPosition;
    float lensq = f.lengthSquared();

    if( lensq > attractionDistance*attractionDistance ) {
        mVelocity += f.normalized() * attractionPower;
    }else
    if( lensq < repellingDistance*repellingDistance ) {
        mVelocity -= f.normalized() * attractionPower;
    }

    updateDirection();
}

bool Animal::collide(Animal *a, float minCollideDistance  )
{
    float distanceSqared = (a->p() - p()).lengthSquared();

    // if both are too far - exit
    if( distanceSqared > (minCollideDistance*minCollideDistance)) {
        return false;
    }

    // if both not approaching to each other
    float distanceNext = ((a->p()+a->v()) - (p()+v())).lengthSquared();
    if( distanceNext > distanceSqared ) {
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

    updateDirection();
    a->updateDirection();

    return true;
}

void Animal::updateSpeed(float maxSpeed, float friction, float rotationFading)
{
    // TODO: use it someday for smooth rotation
    (void) rotationFading;

    if( mVelocity.length() > maxSpeed) {
        mVelocity = mVelocity.normalized() * maxSpeed;
    }

    mVelocity *= (1.0f - friction);
    mPosition += mVelocity;
}

void Animal::putCollar()
{
    mCollar = new Collar();
}

bool Animal::isSideVisible(Animal *a, float maxCosAngle)
{
    // check for visibility range
    QVector2D perp( mDirection.y(), -mDirection.x());
    QVector2D look( a->p() - p() );
    look.normalize();

    return fabs(QVector2D::dotProduct(perp, look)) > maxCosAngle;
}



Animal::Animal(float x, float y )
{
    mPosition = QVector2D(x, y);
    mVelocity = QVector2D(0, 0);
    mDirection = QVector2D(1, 0);

    mBolus = new Bolus(this);
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
