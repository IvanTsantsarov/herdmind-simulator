#include <QDebug>

#include "herd.h"
#include "hardware/tools.h"
#include "animal.h"


#define ANIMAL_MIN_DISTANCE 0.2

void Herd::clear()
{
    foreach(Animal* a, mAnimals) {
        delete a;
    }
    mAnimals.clear();
    mInfoPairs.clear();
}

Herd::Herd(QObject *parent)
    : QObject{parent}
{}

Herd::~Herd()
{
    clear();
}

void Herd::generate(int count, float animalSize, int areaDimeter, int percentageCollars)
{
    clear();

    mAnimalSize = animalSize;

    mCollarsCount = count * percentageCollars / 100;

    float areaRadius = areaDimeter * 0.5;

    qDebug() << "Generating" << count << "herd in radius:" << areaRadius << "and" << mCollarsCount << "collars";

    auto rnd = [&]() {
        return areaDimeter * rand() / RAND_MAX - areaRadius;
    };

    mAnimals.reserve(count);

    for( auto i = 0; i < count; i ++) {
        float x = rnd();
        float y = rnd();
        mAnimals.append(new Animal(x, y));
    }

    for( int i = 0; i < mCollarsCount; i ++) {
        int indexCollar = Tools::rnd(0, count);
        Animal* a = mAnimals[indexCollar];
        while(a->hasCollar()) {
            indexCollar ++;
            if( indexCollar >= count) {
                indexCollar = 0;
            }
            a = mAnimals[indexCollar];
        }
        a->putCollar();
    }
}

void Herd::update( QPointF* attractor,
                  float attractorPower,
                  float attractionDistance,
                  float repellingDistance,
                  float collidingDistance,
                  float maxSpeed,
                  float friction,
                  float rotationFading,
                  float maxTransmitDistance,
                  float maxTransmitAngle )
{
    QVector2D vectAttr = attractor ? QVector2D(*attractor) :  QVector2D();
    float minTransmitAngleCos = cosf(maxTransmitAngle);

    mInfoPairs.clear();

    float animalSzSq = mAnimalSize * mAnimalSize * 0.25f;

    foreach(Animal* a, mAnimals) {

        // clear observers list
        a->clearObservers();
        a->clearObserving();

        // interact
        if( attractor ) {
            a->react(vectAttr, attractorPower, attractionDistance, repellingDistance);
        }

        // collide
        foreach(Animal* other, mAnimals) {
            if( other == a ) {
                continue;
            }

            float distanceOther = a->distanceSq(other);
            QVector2D lookOther = a->p() - other->p();
            lookOther.normalize();

            other->collide(a, collidingDistance );

            if( a->hasCollar() ) {
                AnimalPair ap = AnimalPair(a, other);

                if( distanceOther > maxTransmitDistance*maxTransmitDistance ) {
                    continue;
                }

                if( !a->isSideVisible(other, minTransmitAngleCos ) ) {
                    continue;
                }

                bool isIntersection = false;

                // check weather the animal's bolus is directly visible by the collar
                // without intersection with other animals
                foreach(Animal* intersect, mAnimals) {
                    if( intersect == a || intersect == other) {
                        continue;
                    }

                    if( distanceOther < a->distanceSq(intersect) ) {
                        continue;
                    }

                    if( !a->isSideVisible(intersect, minTransmitAngleCos ) ) {
                        continue;
                    }

                    QVector2D lookIntersect = a->p() - intersect->p();
                    lookIntersect.normalize();

                    float dot = QVector2D::dotProduct(lookOther, lookIntersect);
                    float cross = lookOther.x()*lookIntersect.x() - lookOther.y()*lookIntersect.x();
                    float angle = std::atan2(cross, dot);
                    if (angle < 0)
                        angle += static_cast<float>(M_PI * 2); // make it in range 0..pi

                    if( (angle < M_PI) && (ap.distanceSqToLine(intersect->pt()) < animalSzSq) ) {
                        isIntersection = true;
                        break;
                    }
                }

                // add it to pairs if directly visible
                if(  !isIntersection ) {
                    ap.appendTo( mInfoPairs );
                }
            }
        }
    }

    // update speed
    foreach(Animal* a, mAnimals) {
        a->updateSpeed(maxSpeed, friction, rotationFading);
    }
}



Herd::AnimalPair::AnimalPair(Animal *collarAnimal, Animal *bolusAnimal) :
    mCollarAnimal(collarAnimal), mBolusAnimal(bolusAnimal)
{
    mLine = QLineF(collarAnimal->pt(), bolusAnimal->pt());
}

float Herd::AnimalPair::distanceSqToLine(const QPointF &pt)
{
    // Vector math
    QPointF a = mLine.p1();
    QPointF b = mLine.p2();
    QPointF ap = pt - a;
    QPointF ab = b - a;

    float ab2 = QPointF::dotProduct(ab, ab);
    float ap_ab = QPointF::dotProduct(ap, ab);
    float t = ap_ab / ab2;

    // Clamp t to [0,1] if you want distance to *segment* instead of infinite line
    t = qBound(0.0, t, 1.0);

    QPointF proj = a + t * ab;
    float dx = pt.x() - proj.x();
    float dy = pt.y() - proj.y();
    return dx * dx + dy * dy;
}

void Herd::AnimalPair::appendTo(QList<AnimalPair> &ls)
{
    mBolusAnimal->addObserver(mCollarAnimal);
    mCollarAnimal->addObserving(mBolusAnimal);
    ls.append(*this);
}
