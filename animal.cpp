#include <QDebug>
#include "defines.h"
#include "animal.h"
#include "herd.h"
#include "simtools.h"
#include "hardware/bolus/bolus.h"
#include "hardware/collar/collar.h"


#define MIN_FLOAT 1e-6f

void Animal::updateRotationAngleTarget()
{
    mRotationAngleTarget = qAtan2(mDirection.y(), mDirection.x()); // -pi..+pi
    if (mRotationAngleTarget < 0)
        mRotationAngleTarget += static_cast<float>(M_PI * 2); // make it in range 0..pi
}

void Animal::updateSpeedAndDirection()
{
    mSpeed = mVelocity.length();

    if( mSpeed > 0.00000001f ) {
        mDirection = mVelocity / mSpeed;
    }

    if( mSpeed > ANIMAL_MAX_SPEED ) {
        mSpeed = ANIMAL_MAX_SPEED;
        mVelocity = mDirection * mSpeed;
    }
}

void Animal::setState(State newState) {
    mStatePrev = mState;
    mState = newState;
    mStateMsec = mHerd->msec();
}

quint64 Animal::stateMsec()
{
    return mHerd->msec() - mStateMsec;
}



float Animal::distanceSq(Animal *other)
{
    float dx = mPosition.x() - other->mPosition.x();
    float dy = mPosition.y() - other->mPosition.y();
    return dx*dx + dy*dy;
}

void Animal::updateRunning(const QVector2D &p, float attractionPower, float attractionDistance, float repellingDistance, float friction )
{
    QVector2D f = QVector2D(p) - mPosition;
    float lensq = f.lengthSquared();

    if( lensq > attractionDistance*attractionDistance ) {
        mVelocity += f.normalized() * attractionPower;
    }else
    if( lensq < repellingDistance*repellingDistance ) {
        mVelocity -= f.normalized() * attractionPower;
    }

    updateSpeedAndDirection();
    updateRotationAngleTarget();
    mRotationAngle = mRotationAngleTarget;
}

void Animal::run(bool is)
{
    if( is ) {
        if( !isRunning() ) {
            setState(State::running);
        }
    }else {
        if( isRunning() ) {
            setState(State::stopping);
            mVelocity = mDirection * ANIMAL_WALKING_SPEED;
        }
    }
}


void Animal::lookAt(const QVector2D &destination)
{
    QVector2D delta = destination - mPosition;
    float len = delta.length();
    if( len < 0.00000001f) {
        return;
    }
    mDirection = delta / len;
    updateRotationAngleTarget();
    // qDebug() << mRotationAngleTarget << mRotationAngle;
}


// return true if still walking
bool Animal::goTo(const QVector2D &destination, float speed, float arrivingDistance)
{
    mDestination = destination;
    mSpeed = mSpeed > ANIMAL_MAX_SPEED ? ANIMAL_MAX_SPEED : speed;
    mArrivingDistance = arrivingDistance;

    lookAt(destination);
    mVelocity = mDirection * mSpeed;
    setState(State::going);
    return true;
}

bool Animal::walkTo(const QVector2D &destination)
{
    return goTo( destination, ANIMAL_WALKING_SPEED, LAWN_RADIUS );
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

    // if it's stoping stop it
    if( isStopping() || isResting() || isSitting() ) {
        mVelocity = QVector2D(0.0f, 0.0f);
        mSpeed = 0.0f;
        return true;
    }

    if( !isRunning() ) {
        // change the direction
        QVector2D v1 = SimTools::rotated(mDirection, ANIMAL_AVOID_ROTATION_ANGLE);
        QVector2D v2 = SimTools::rotated(mDirection,-ANIMAL_AVOID_ROTATION_ANGLE);
        mDirection = QVector2D::dotProduct(other->mDirection, v1) < 0.0f ? v1 : v2;
        mVelocity = mDirection * mSpeed;

        setState(State::avoiding);
        mObstacle = other;
        dettach();

        // kick other animal if comming from behind
        if( QVector2D::dotProduct( -vecDist.normalized(), other->mDirection ) < 0.0f ) {
            other->kick();
        }

        updateRotationAngleTarget();
    }

    return true;
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
    return QVector2D::dotProduct(mDirection, look) > maxCosAngle;
}


bool Animal::isSideVisible(Animal *a, float maxCosAngle)
{
    // check for visibility range
    QVector2D perp( mDirection.y(), -mDirection.x());
    QVector2D look( a->p() - p() );
    look.normalize();

    return fabs(QVector2D::dotProduct(perp, look)) > maxCosAngle;
}



Animal::Animal(Herd* herd, float x, float y , float grazingCapacity) : mHerd(herd)
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

void Animal::kick()
{
    if( !isGoing() && !isResting() ) {
        mSpeed = ANIMAL_WALKING_SPEED;
        setState(State::stopping);
        dettach();
    }
}

bool Animal::isCloseToDestination()
{
    QVector2D f = QVector2D(mDestination) - mPosition;
    return QVector2D::dotProduct( f, f ) < (mArrivingDistance*mArrivingDistance);
}

bool Animal::isArrived()
{
    if( !isSitting() ) {
        return false;
    }

    return isCloseToDestination();
}

void Animal::attach(Meadow::Lawn *newLawn)
{
    if( mLawn ) {
        mLawn->dettach(this);
    }

    if( newLawn ) {
        if( newLawn->hasSpace() ) {
            newLawn->attach(this);
            mLawn = newLawn;
        }
    }else {
        // nullptr
        mLawn = newLawn;
    }
}

bool Animal::graze()
{
    if( !mLawn || mLawn->isDepleted() ) {
        return false;
    }

    setState(State::grazing);

    return true;
}

QString Animal::info()
{
    float kg = mLawn ? mLawn->kg() : 0.0f;

    return QString("Ptr:0x%1\nV:%2,%3\nA:%4,%5\nStamina:%6\nState:%7 %8\nLawn:0x%9\nKg=%10\nAnimals:%11")
        .arg(reinterpret_cast<quint64>(this), 0, 16)
        .arg( mVelocity.x(), 0, 'f', 2)
        .arg( mVelocity.y(), 0, 'f', 2)

        .arg( mRotationAngleTarget, 0, 'f', 2)
        .arg( mRotationAngle, 0, 'f', 2)

        .arg( mStamina, 0, 'f', 2)

        .arg( static_cast<int>(mState) )
        .arg( currentStateString() )


        .arg(reinterpret_cast<quint64>(mLawn), 0, 16)
        .arg(kg, 0, 'f', 2 )
        .arg(mLawn ? mLawn->animalsCount() : 0 );
}

QString Animal::jsonInfo()
{
    QString result = "";
    if( hasCollar() ) {
        result.append( mCollar->jsonInfo() );
        result.append(",");
    }

    result.append( mBolus->jsonInfo() );
    return result;
}

// return true if lawn is depleted


void Animal::updateBehavior(float tickSeconds, float friction, float rotationFading)
{
    float angleDelta = fmod(mRotationAngleTarget - mRotationAngle, 2*M_PI);

    if( angleDelta < -M_PI) angleDelta += 2*M_PI;
    else
        if( angleDelta > M_PI) angleDelta -= 2*M_PI;

    mRotationAngle = fmod( mRotationAngle + rotationFading * angleDelta, 2*M_PI);

    if( isGrazing() ) {
        if( !mLawn || mLawn->graze(tickSeconds * mGrazingCapacity / 60) ) {
            setState(State::sitting);
        }
    }

    if( isAvoiding() ) {
        if( distanceSq(mObstacle) > ANIMAL_AVOID_DISTANCE*ANIMAL_AVOID_DISTANCE ) {
            walkTo(mDestination);
            if( isCloseToDestination()) {
                setState(State::stopping);
            }
        }
    }

    if( isGoing() ) {
        if( isCloseToDestination() ) {
            setState(State::stopping);
        }
    }


    if( isStopping() || isResting() ) {
        mSpeed -= friction * tickSeconds;
    }

    // stop if going too slow
    if( mSpeed < (ANIMAL_MIN_SPEED * ANIMAL_MIN_SPEED) ) {

        mVelocity = QVector2D( 0.0f, 0.0f);
        mSpeed = 0.0f;

        if( isMoving() ) {
            setState(State::sitting);
        }
    }

    if( isMoving() ) {
        mVelocity = mDirection * mSpeed;

        mStamina -= ANIMAL_STAMINA_STEP * tickSeconds;

        if( mStamina < ANIMAL_STAMINA_MIN ) {
            mStamina = 0.0f;
            if( !isRunning() ) {
                setState(State::resting);
            }
        }
    }else {
        mStamina += ANIMAL_STAMINA_STEP * tickSeconds;

        if( mStamina > ANIMAL_STAMINA_MAX ) {
            mStamina = ANIMAL_STAMINA_MAX;

            if( isResting() ) {
                setState(State::sitting);
            }
        }
    }
}

void Animal::updateCommon(float tickSeconds)
{
    if( !isSitting() ) {
        mPosition += mVelocity;
    }

    int msec = tickSeconds * 1000;

    if( mBolus ) {
        mBolus->updateSendingSimulation( msec );
    }

    if( mCollar ) {
        mCollar->updateSendingSimulation( msec );
    }
}
