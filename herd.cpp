#include "herd.h"
#include "animal.h"

#define ANIMAL_MIN_DISTANCE 0.2

void Herd::clear()
{
    foreach(Animal* a, mAnimals) {
        delete a;
    }
    mAnimals.clear();
}

Herd::Herd(QObject *parent)
    : QObject{parent}
{}

void Herd::generate(int count, int areaDimeter)
{
    clear();

    float areaDimeterHalf = areaDimeter * 0.5;

    auto rnd = [&]() {
        return areaDimeter * rand() / RAND_MAX - areaDimeterHalf;
    };

    mAnimals.reserve(count);

    for( auto i = 0; i < count; i ++) {
        float x = rnd();
        float y = rnd();
        mAnimals.append(new Animal(x, y, true, true));
    }
}

void Herd::update( QPointF* attractor,
                  float attractorPower,
                  float attractionDistance,
                  float repellingDistance,
                  float collidingDistance,
                  float maxSpeed,
                  float friction )
{
    QVector2D vectAttr = attractor ? QVector2D(*attractor) :  QVector2D();

    foreach(Animal* a, mAnimals) {

        // interact
        if( attractor ) {
            a->interact(vectAttr, attractorPower, attractionDistance, repellingDistance);
        }

        // collide
        foreach(Animal* other, mAnimals) {
            if( other == a ) {
                continue;
            }
            other->collide(a, collidingDistance );
        }
    }

    // apply
    foreach(Animal* a, mAnimals) {
        a->updateSpeed(maxSpeed, friction);
    }
}

