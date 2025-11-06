#ifndef HERD_H
#define HERD_H

#include <QObject>
#include <QVector2D>
#include <QLineF>

class Animal;
class Shepherd;
class Meadow;

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


private:
    Q_OBJECT

    quint64 mMSec = 0;
    quint64 mLastUpdateMsec = 0;

    QVector<Animal*> mAnimals, mCollars;

    PairsListBC mPairsBC;

    float mAnimalSize = 0;
    float mAnimalHalfSizeSquared = 0;

    void clear();

    bool checkTransmitVisibility(AnimalPair& ap, float maxDistanceSq, float minTransmitAngleCos);

    Shepherd* mShepherd = nullptr;
    bool mIsShepherdActive = false;

    bool processCollision(float collidingDistance);
public:
    explicit Herd(QObject *parent = nullptr);
    ~Herd();
    void generate(int count,
                  int areaDimeter,
                  int percentageCollars,
                  float animalSize,
                  float grazingCapacity );

    void update(quint64 millissec,
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

    inline quint64 msec(){ return mMSec; }

    inline Animal* animal(int index){ return mAnimals[index]; }
    inline int animalsCount(){ return mAnimals.count(); }
    PairsListBC& pairs() { return mPairsBC; };
    inline int count(){ return mAnimals.count(); }
    int collarsCount(){ return mCollars.count(); }

    void activateShepherd(bool is) { mIsShepherdActive = is; };
    bool isShepherdActive() { return mShepherd && mIsShepherdActive; };
    QPointF shepherdPos();;

signals:
};

#endif // HERD_H
