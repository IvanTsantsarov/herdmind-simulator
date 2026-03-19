#ifndef HERD_H
#define HERD_H

#include <QObject>
#include <QVector2D>
#include <QLineF>
#include "hardware/loradev.h"

class Animal;
class Shepherd;
class Meadow;
class LoraDev;

class Herd : public QObject
{
public:

    class AnimalPair {
        Animal* mCollarAnimal  = nullptr;
        Animal* mBolusAnimal = nullptr;
        QLineF mLine;
    public:
        AnimalPair( Animal* collarAnimal, Animal* bolusAnimal);
        inline Animal* collarAnimal(){ return mCollarAnimal;};
        inline Animal* bolusAnimal() { return mBolusAnimal; }
        float distanceSqToLine(const QPointF& pt);
        void appendTo(QList<AnimalPair>& ls);
    };

    typedef QList<AnimalPair> PairsListBC;

    bool mIsEnabledGrazing = true;
    bool mIsEnabledHerding = true;

    QList<LoraDev*> gatherDevices();


private:
    Q_OBJECT

    QVector<Animal*> mAnimals, mCollars;

    int mMalesCount = 0;

    PairsListBC mPairsBC;

    float mAnimalSize = 0;
    float mAnimalHalfSizeSquared = 0;

    void clear();

    bool checkTransmitVisibility(AnimalPair& ap, float maxDistanceSq, float minTransmitAngleCos);

    Shepherd* mShepherd = nullptr;
    bool mIsShepherdActive = false;

    Meadow* mMeadow = nullptr;

    bool processCollision(float collidingDistance);
    float beforeGeneration(int areaDimeter, float animalSize);
public:
    explicit Herd(QObject *parent = nullptr);
    ~Herd();

    inline int malesCount(){ return mMalesCount;}
    inline int femalesCount(){ return mAnimals.count() - mMalesCount;}

    bool loadFromFile(int areaDimeter, float animalSize);

    bool generate(int count,
                  int areaDimeter,
                  int percentageCollars, int percentageMales,
                  float animalSize,
                  float grazingCapacity );

    bool load(  const QString& jsonPath,
                int areaDimeter,
                float animalSize,
                float grazingCapacity );


    void update(float tickSeconds,
                Meadow *meadow,
                QPointF *attractor,
                bool isCorrectCollision,
                float attractorPower,
                float attractionDistance,
                float repellingDistance,
                float collidingDistance,
                float maxSpeed ,
                float friction,
                float rotationFading,
                float maxTransmitDistance,
                float maxTransmitAngle
                );

    inline Animal* animal(int index){ return mAnimals[index]; }
    inline Meadow* meadow(){ return mMeadow; }


    inline int animalsCount(){ return mAnimals.count(); }
    PairsListBC& pairs() { return mPairsBC; };
    inline int count(){ return mAnimals.count(); }
    int collarsCount(){ return mCollars.count(); }

    void activateShepherd(bool is) { mIsShepherdActive = is; };
    bool isShepherdActive() { return mShepherd && mIsShepherdActive; };
    QPointF shepherdPos();

    QString jsonAnimalsList(bool isDevicesList);

    void storeLists();

signals:
};

#endif // HERD_H
