#ifndef ANIMAL_H
#define ANIMAL_H

#include <QVector2D>
#include <QList>

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

    void updateDirection();

    // count of the animals with collars that sees this animal
    QList<Animal*> mObservers;
    QList<Animal*> mObserving;

    void onThisBolusSent(void* package);
    void onThisCollarSent(void* package);
    void onOtherBolusData(void* package, Animal* from);

    // how many readings from other boluses
    uint mReadings = 0;

public:

    inline QVector2D& p(){ return mPosition; }
    inline QPointF pt(){ return QPointF(mPosition.x(), mPosition.y()); }
    inline QVector2D& v(){ return mVelocity; }
    float distanceSq(Animal* other);
    void react(const QVector2D &p,
                  float attractionPower,
                  float attractionDistance,
                  float repellingDistance);

    // return true if info from bolus is sent to the animal
    bool collide(Animal* a, float minCollideDistance);
    void updateSpeed(float maxSpeed, float friction, float rotationFading);
    inline float rotationAngle(){ return mRotationAngle; }

    void putCollar();
    bool hasCollar() const { return nullptr != mCollar; }
    bool isSideVisible( Animal* a, float maxCosAngle );

    void clearObservers(){ mObservers.clear(); }
    void addObserver(Animal* a){ mObservers.append(a); }
    int observersCount(){ return mObservers.count(); }

    void clearObserving(){ mObserving.clear(); }
    void addObserving(Animal* a){ mObserving.append(a); }
    int observingCount(){ return mObserving.count(); }

    Animal(float x, float y );
    ~Animal();

    uint readings(){ return mReadings; }
};

#endif // ANIMAL_H
