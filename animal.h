#ifndef ANIMAL_H
#define ANIMAL_H

#include <QVector2D>
#include <QList>
#include "meadow.h"

class Bolus;
class Collar;

class Animal {

    friend class Bolus;
    friend class Collar;

    Bolus* mBolus = nullptr;
    Collar* mCollar = nullptr;

    QVector2D mPosition;
    QVector2D mVelocity;
    QVector2D mDirection;
    float mRotationAngle = 0.0f;
    float mRotationAngleTarget = 0.0f;

    void updateDirection();

    // count of the animals with collars that sees this animal
    QList<Animal*> mObservers;
    QList<Animal*> mObserving;

    void onThisBolusSent(void* package);
    void onThisCollarSent(void* package);
    void onOtherBolusData(void* package, Animal* from);

    Meadow::Lawn* mLawn = nullptr;

    // how many readings from other boluses
    uint mReadings = 0;

    float mGrazingCapacity; // in kilograms per 24 hours

public:

    inline QVector2D& p(){ return mPosition; }
    inline QPointF pt(){ return QPointF(mPosition.x(), mPosition.y()); }
    inline QVector2D& v(){ return mVelocity; }
    float distanceSq(Animal* other);
    void react(const QVector2D &p,
                  float attractionPower,
                  float attractionDistance,
                  float repellingDistance);


    bool isMoving();
    bool walk(const QVector2D& destination, float speed, float arrivingDistance);

    // return true if info from bolus is sent to the other
    bool collide(Animal* other, float minCollideDistance);
    void updateSpeed(float maxSpeed, float friction, float rotationFading);
    inline float rotationAngle(){ return mRotationAngle; }
    inline float rotationAngleTarget(){ return mRotationAngleTarget; }

    void putCollar();
    bool hasCollar() const { return nullptr != mCollar; }
    bool isSideVisible(Animal *a, float maxCosAngle);
    bool isAhead(Animal* animal, float maxCosAngle );


    void clearObservers(){ mObservers.clear(); }
    void addObserver(Animal* a){ mObservers.append(a); }
    int observersCount(){ return mObservers.count(); }

    void clearObserving(){ mObserving.clear(); }
    void addObserving(Animal* a){ mObserving.append(a); }
    int observingCount(){ return mObserving.count(); }

    Animal(float x, float y, float grazingCapacity );
    ~Animal();

    uint readings(){ return mReadings; }

    void attach(Meadow::Lawn *newLawn = nullptr);
    inline Meadow::Lawn* lawn(){ return mLawn; }

    // return true if lawn depleted
    bool graze(float timeFactor) { return mLawn ? mLawn->graze(timeFactor * mGrazingCapacity) : true; }

    QString info();
};

#endif // ANIMAL_H
