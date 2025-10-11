#ifndef ANIMAL_H
#define ANIMAL_H

#include <QVector2D>

class Bolus;
class Collar;

class Animal {

    Bolus* mBolus = nullptr;
    Collar* mCollar = nullptr;

    QVector2D mPosition;
    QVector2D mVelocity;
    QVector2D mDirection;
    float mRotationAngle = 0.0f;

    void updateDirection();

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
    inline QPointF point(){ return QPointF(mPosition.x(), mPosition.y()); }

    void putCollar();
    bool hasCollar() const { return nullptr != mCollar; }
    bool isSideVisible( Animal* a, float maxCosAngle );

    Animal(float x, float y );
    ~Animal();
};

#endif // ANIMAL_H
