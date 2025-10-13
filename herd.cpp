#include <QDebug>

#include "herd.h"
#include "shepherd.h"
#include "hardware/tools.h"
#include "animal.h"


#define ANIMAL_MIN_DISTANCE 0.2

void Herd::clear()
{
    foreach(Animal* a, mAnimals) {
        delete a;
    }
    mAnimals.clear();
    mCollars.clear();
    mInfoPairs.clear();
}

Herd::Herd(QObject *parent)
    : QObject{parent}
{

}

Herd::~Herd()
{
    if( mShepherd ) {
        delete mShepherd;
        mShepherd = nullptr;
    }
    clear();
}

void Herd::generate(int count, float animalSize, int areaDimeter, int percentageCollars)
{
    srand(time(NULL));

    clear();

    mAnimalSize = animalSize;
    mAnimalHalfSizeSquared = mAnimalSize * mAnimalSize * 0.25f;

    int collarsCount = count * percentageCollars / 100;

    float areaRadius = areaDimeter * 0.5;

    qDebug() << "Generating" << count << "herd in radius:" << areaRadius << "and" << collarsCount << "collars";

    mShepherd = new Shepherd(0.001f, areaRadius);

    // fill animals array
    mAnimals.reserve(count);
    for( auto i = 0; i < count; i ++) {
        float x = Tools::rnd(-areaRadius, areaRadius);
        float y = Tools::rnd(-areaRadius, areaRadius);
        mAnimals.append(new Animal(x, y));
    }

    // fill animals with collar array
    mCollars.reserve(collarsCount);
    for( int i = 0; i < collarsCount; i ++) {
        int indexCollar = Tools::rnd(0, count);
        Animal* animal = mAnimals[indexCollar];
        while(animal->hasCollar()) {
            indexCollar ++;
            if( indexCollar >= count) {
                indexCollar = 0;
            }
            animal = mAnimals[indexCollar];
        }
        animal->putCollar();
        mCollars.append(animal);
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

    // process attractor force if available
    if( attractor ) {
        foreach(Animal* animal, mAnimals) {
            animal->react(vectAttr, attractorPower, attractionDistance, repellingDistance);
        }
    }

    // process shepherd if active
    if( mIsShepherdActive ) {
        QPointF pt = mShepherd->step();
        QVector2D vectorShepherd(pt);
        foreach(Animal* animal, mAnimals) {
            animal->react(vectorShepherd, attractorPower, attractionDistance, repellingDistance);
        }
    }

    // process collision
    foreach(Animal* animal, mAnimals) {

        // collide
        foreach(Animal* otherAnimal, mAnimals) {
            // avoid collision with itself
            if( otherAnimal == animal ) {
                continue;
            }

            otherAnimal->collide(animal, collidingDistance );
        }
    }

    // update speed after attraction and collision
    foreach(Animal* animal, mAnimals) {
        animal->updateSpeed(maxSpeed, friction, rotationFading);
    }


    float maxTransmitDistanceSq = maxTransmitDistance*maxTransmitDistance;


    // clear statistics lists
    foreach(Animal* animal, mAnimals) {
        animal->clearObservers();
        animal->clearObserving();
    }

    // update boluses visibilities for all animals with collars
    foreach(Animal* animal, mCollars) {

        foreach(Animal* otherAnimal, mAnimals) {

            float distanceOtherSq = animal->distanceSq(otherAnimal);

            if( distanceOtherSq > maxTransmitDistanceSq ) {
                continue;
            }

            if( !animal->isSideVisible(otherAnimal, minTransmitAngleCos ) ) {
                continue;
            }

            // add it to pairs if directly visible
            AnimalPair ap(animal, otherAnimal);
            if(  checkTransmitVisibility(ap, distanceOtherSq, minTransmitAngleCos) ) {
                ap.appendTo( mInfoPairs );
            }
        }
    }
}

QPointF Herd::shepherdPos()
{
    if( !mShepherd) {
        return QPointF(0, 0);
    }

    return mShepherd->lastPos();
}


bool Herd::checkTransmitVisibility(AnimalPair& ap, float maxDistanceSq, float minTransmitAngleCos)
{
    QVector2D look = ap.collarAnimal()->p() - ap.bolusAnimal()->p();
    look.normalize();

    // check weather the animal's bolus is directly visible by the collar
    // without intersection with other animals
    foreach(Animal* intersect, mAnimals) {
        // skip both animals from the pair
        if( intersect == ap.collarAnimal() || intersect == ap.bolusAnimal()) {
            continue;
        }

        // check if it's too far
        if( maxDistanceSq < ap.collarAnimal()->distanceSq(intersect) ) {
            continue;
        }

        // check angle of view
        if( !ap.collarAnimal()->isSideVisible(intersect, minTransmitAngleCos ) ) {
            continue;
        }

        // test for the intersection with the line of vision
        QVector2D lookIntersect = ap.collarAnimal()->p() - intersect->p();
        lookIntersect.normalize();

        float dot = QVector2D::dotProduct(look, lookIntersect);
        float cross = look.x()*lookIntersect.x() - look.y()*lookIntersect.x();
        float angle = std::atan2(cross, dot);
        if (angle < 0)
            angle += static_cast<float>(M_PI * 2); // make it in range 0..pi

        if( (angle < M_PI) && (ap.distanceSqToLine(intersect->pt()) < mAnimalHalfSizeSquared) ) {
            // animal pair is obscured by this animal
            return false;
        }
    }

    return true;
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
