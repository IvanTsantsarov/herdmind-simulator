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
    float mRotationAngle = 0.0f;
    float mRotationAngleGoal = 0.0f;


public:

    inline QVector2D& p(){ return mPosition; }
    inline QVector2D& v(){ return mVelocity; }
    void interact(const QVector2D &p, float attractionPower, float attractionDistance, float repellingDistance);
    bool collide(Animal* a, float minDistance );
    void updateSpeed(float maxSpeed, float friction, float rotationFading);
    inline float rotationAngle(){ return mRotationAngle; }
    inline QPointF point(){ return QPointF(mPosition.x(), mPosition.y()); }

    Animal(float x, float y, bool isBolus, bool isCollar);
    ~Animal();
};

#endif // ANIMAL_H
