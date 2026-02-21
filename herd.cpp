#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include "herd.h"
#include "hardware/bolus/bolus.h"
#include "simtools.h"
#include "shepherd.h"
#include "hardware/tools.h"
#include "animal.h"
#include "defines.h"

#define ANIMAL_MIN_DISTANCE 0.2

void Herd::clear()
{
    foreach(Animal* a, mAnimals) {
        delete a;
    }
    mAnimals.clear();
    mCollars.clear();
    mPairsBC.clear();
}

Herd::Herd(QObject *parent)
    : QObject{parent}
{
    // mIsEnabledGrazing = false; // trash
    // mIsEnabledHerding = false; // trash
}

Herd::~Herd()
{
    if( mShepherd ) {
        delete mShepherd;
        mShepherd = nullptr;
    }
    clear();
}


bool Herd::load(const QString &jsonPath, int areaDimeter, float animalSize, float grazingCapacity )
{
    float areaRadius = beforeGeneration( areaDimeter, animalSize);

    bool isOK;
    QByteArray content = gSimTools->fileRead(jsonPath, &isOK);
    if( !isOK ) {
        qWarning() << "Error reading file:" << ANIMALS_LIST_FILE;
        return false;
    }

    QJsonArray jarr = QJsonDocument::fromJson(content).array();

    mAnimals.reserve(jarr.size());
    foreach(auto jsonElement, jarr) {
        QJsonObject jobj = jsonElement.toObject();

        float x = Tools::rnd(-areaRadius, areaRadius);
        float y = Tools::rnd(-areaRadius, areaRadius);

        Animal* animal = new Animal(this, jobj["male"].toBool(), jobj["name"].toString(), x, y, grazingCapacity );

        if( jobj.contains("bolus") ) {
            animal->putBolus()->setFromJson(jobj["bolus"].toObject());
        };


        if( jobj.contains("collar") ) {
            animal->putCollar()->setFromJson(jobj["collar"].toObject());
            mCollars.append(animal);
        };

        mAnimals.append(animal);
    }

    return true;
}


float Herd::beforeGeneration( int areaDimeter, float animalSize )
{
    srand(time(NULL));

    clear();

    mAnimalSize = animalSize;
    mAnimalHalfSizeSquared = mAnimalSize * mAnimalSize * 0.25f;
    float areaRadius = areaDimeter * 0.5;
    mShepherd = new Shepherd(0.001f, areaRadius);

    return areaRadius;
}

// returns true if successful load from saved animals list
bool Herd::generate(int count,
                    int areaDimeter,
                    int percentageCollars,
                    int percentageMales,
                    float animalSize,
                    float grazingCapacity)
{
    float areaRadius = beforeGeneration( areaDimeter, animalSize);

    int collarsCount = count * percentageCollars / 100;

    qDebug() << "Generating" << count << "herd in radius:" << areaRadius << "and" << collarsCount << "collars";

    mMalesCount = percentageMales * count;
    if( mMalesCount < 1 ) mMalesCount = 1;

    if( mMalesCount > MALE_NAMES_COUNT) {
        mMalesCount = MALE_NAMES_COUNT;
    }

    // fill animals array
    mAnimals.reserve(count);

    int malesGenerated = 0;

    QList<int> maleNames = Animal::namesIndices(true);
    QList<int> femaleNames = Animal::namesIndices(false);

    for( auto i = 0; i < count; i ++) {

        int nameIndex;

        bool isMale = malesGenerated >= mMalesCount ? false : true;

        if( isMale ) {
            nameIndex = maleNames.size() > 1 ? gSimTools->gen(maleNames.size()-1) : 0;
            maleNames.removeAt(nameIndex);
            malesGenerated ++;
        }else {
            nameIndex = femaleNames.size() > 1 ? gSimTools->gen(femaleNames.size()-1) : 0;
            femaleNames.removeAt(nameIndex);
        }

        float x = Tools::rnd(-areaRadius, areaRadius);
        float y = Tools::rnd(-areaRadius, areaRadius);
        Animal* animal = new Animal(this, isMale, nameIndex, x, y, grazingCapacity);
        mAnimals.append(animal);
        animal->putBolus(); /// TODO: put only ot specified percentage
        processCollision(animalSize * 2.0f);
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

    // If cannot load params from animals list - save animals list
    gSimTools->fileWrite(ANIMALS_LIST_FILE, jsonAnimalsList(false).toUtf8(), true);
    gSimTools->fileWrite(DEVICES_LIST_FILE, jsonAnimalsList(true).toUtf8(), true);

    return true;
}


bool Herd::processCollision(float collidingDistance)
{
    // process collision
    bool isCollision = false;
    foreach(Animal* animal, mAnimals) {
         foreach(Animal* otherAnimal, mAnimals) {
            // avoid collision with itself
            if( otherAnimal == animal ) {
                continue;
            }

            if( otherAnimal->collide(animal, collidingDistance ) ) {
                isCollision = true;
            }
        }
    }

    return isCollision;
}

void Herd::update( quint64 millissec,
                  Meadow* meadow,
                  QPointF* attractor,
                  bool isCorrectCollision,
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
    mMSec = millissec;
    double msecDelta = mMSec - mLastUpdateMsec;
    float tickSeconds = msecDelta / 1000.0f;
    float tickDays = tickSeconds / (24.0 * 60.0 * 60.0);

    float minTransmitAngleCos = cosf(maxTransmitAngle);

    mPairsBC.clear();

    bool mustRun = mIsEnabledHerding && (attractor || mIsShepherdActive);

    foreach(Animal* animal, mAnimals) {
        animal->run( mustRun );
    }


    if( !mustRun ) {
        ////////////////////////////////////////////////////////////////////
        /// Grazing
        ////////////////////////////////////////////////////////////////////

        if( !attractor && !mIsShepherdActive) {

            foreach(Animal* animal, mAnimals) {
                if( !animal->lawn() && !animal->isMoving() && !animal->isResting() ) {
                    Meadow::Lawn* lawn = meadow->bestAvailable(animal->pt());
                    if( lawn ) {
                        animal->walkTo(QVector2D(lawn->pos()));
                        animal->attach(lawn);
                    }
                }

                if( animal->lawn() && !animal->isGrazing() ) {
                    if( animal->isArrived() ) {
                        if( !animal->graze() ) {
                            animal->dettach();
                        }
                    }else
                    // if it's not arrived
                    if( !animal->isMoving()) {
                        animal->dettach();
                    }else {
                        Meadow::Lawn* lawn = meadow->lawn(animal->pt());
                        if( lawn && !lawn->isDepleted()) {
                            // if there is a lawn under animal's feet just go and graze it
                            animal->walkTo(QVector2D(lawn->pos()));
                            animal->attach(lawn);
                        }
                    }
                }

                animal->updateBehavior(tickSeconds, friction, rotationFading);
            }
        }
    }else
    ////////////////////////////////////////////////////////////////////
    /// Running
    ////////////////////////////////////////////////////////////////////
    {
        QVector2D shepherdPosition;
        if( mIsShepherdActive ) {
            QPointF pt = mShepherd->step();
            shepherdPosition = QVector2D(pt);
        }

        // process attractor force if available
        foreach(Animal* animal, mAnimals) {
            if( animal->lawn() ) {
                animal->dettach();
            }

            if( attractor ) {
                animal->updateRunning(QVector2D(*attractor), attractorPower,
                                      attractionDistance, repellingDistance, friction);
            }

            if( mIsShepherdActive ) {
                animal->updateRunning(shepherdPosition, attractorPower,
                                      attractionDistance, repellingDistance, friction);
            }

        }
    }

    // update position and bolus
    foreach(Animal* animal, mAnimals) {
        animal->updateCommon(tickSeconds);
    }

    // Precision collision is disabled - too expensinve
    if( isCorrectCollision ) {
        // process collision
        int collisionCounter = 0;
        while( processCollision(collidingDistance) ) {
            if( ++collisionCounter > mAnimals.count() ) {
                break;
            };
        }
    }else {
        processCollision(collidingDistance);
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

            if( !animal->isAhead(otherAnimal, minTransmitAngleCos ) ) {
                continue;
            }

            // add it to pairs if directly visible
            AnimalPair ap(animal, otherAnimal);
            //if(  checkTransmitVisibility(ap, distanceOtherSq, minTransmitAngleCos) ) {
                ap.appendTo( mPairsBC );
            //}

        }
    }

    mLastUpdateMsec = mMSec;
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


QString Herd::jsonAnimalsList( bool isDevicesList )
{
    QString result = "[";
    bool isFirst = true;
    foreach(Animal* animal, mAnimals) {
        if( !isFirst) {
            result.append(",");
        }
        result.append( animal->jsonInfo(isDevicesList) );
        isFirst = false;
    }

    result.append("]");

    return result;
}
