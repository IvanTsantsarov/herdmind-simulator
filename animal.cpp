#include "animal.h"
#include "hardware/bolus/bolus.h"
#include "hardware/collar/collar.h"


#define MIN_FLOAT 1e-6f

void Animal::updateDirection()
{
    mDirection = mVelocity.normalized();
    mRotationAngleTarget = qAtan2(mDirection.y(), mDirection.x()); // -pi..+pi

//    if (mRotationAngleTarget < 0)
//        mRotationAngleTarget += static_cast<float>(M_PI * 2); // make it in range 0..pi

}

// When bolus from this animal sends a package
void Animal::onThisBolusSent(void *package)
{
    foreach( Animal* animal, mObservers) {
        animal->onOtherBolusData(package, this);
    }
}

void Animal::onThisCollarSent(void *package)
{

}

// Count a package from other animal bolus
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

bool Animal::collide(Animal *other, float minCollideDistance  )
{
    QVector2D vecDist = other->p() - p();
    float distanceSqared = vecDist.lengthSquared();


    // if both are too far - exit
    if( distanceSqared > (minCollideDistance*minCollideDistance)) {
        return false;
    }

    // if both not approaching to each other
    float distanceNext = ((other->p()+other->v()) - (p()+v())).lengthSquared();
    if( distanceNext > distanceSqared ) {
        return false;
    }

    // correct the position
    mPosition = other->mPosition - vecDist.normalized() * minCollideDistance * 1.00001;

    // change the direction
    other->mVelocity = mVelocity = (mVelocity + other->mVelocity) * 0.5f;
    updateDirection();
    other->updateDirection();

    return true;
}

void Animal::updateSpeed(float maxSpeed, float friction, float rotationFading)
{
    // TODO: use it someday for smooth rotation
    (void) rotationFading;

    if( mVelocity.length() > maxSpeed) {
        mVelocity = mVelocity.normalized() * maxSpeed;
    }

    mRotationAngle = mRotationAngle + rotationFading * (mRotationAngleTarget - mRotationAngle);
    mVelocity *= (1.0f - friction);
    mPosition += mVelocity;
}

void Animal::putCollar()
{
    mCollar = new Collar();
}

bool Animal::isAhead(Animal *a, float maxCosAngle)
{
    // check for visibility range
    QVector2D look( a->p() - p() );
    look.normalize();
    return fabs(QVector2D::dotProduct(mDirection, look)) > maxCosAngle;
}


bool Animal::isSideVisible(Animal *a, float maxCosAngle)
{
    // check for visibility range
    QVector2D perp( mDirection.y(), -mDirection.x());
    QVector2D look( a->p() - p() );
    look.normalize();

    return fabs(QVector2D::dotProduct(perp, look)) > maxCosAngle;
}



Animal::Animal(float x, float y , float grazingCapacity)
{
    mPosition = QVector2D(x, y);
    mVelocity = QVector2D(0, 0);
    mDirection = QVector2D(1, 0);
    mGrazingCapacity = grazingCapacity;

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

void Animal::attach(Meadow::Lawn *newLawn)
{
    if( mLawn ) {
        mLawn->deattach(this);
    }

    mLawn->attach(this);
    mLawn = newLawn;
}

void Animal::graze()
{
    if( mLawn) {
        mLawn->graze(mGrazingCapacity);
    }
}
