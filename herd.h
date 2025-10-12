#ifndef HERD_H
#define HERD_H

#include <QObject>
#include <QVector2D>
#include <QLineF>

class Animal;

class Herd : public QObject
{
public:

    class AnimalPair {
        Animal* mCollarAnimal  = nullptr;
        Animal* mBolusAnimal = nullptr;
        QLineF mLine;
    public:
        AnimalPair( Animal* collarAnimal, Animal* bolusAnimal);
        Animal* collarAnimal(){ return mCollarAnimal;};
        Animal* bolusAnimal() { return mBolusAnimal; }
        float distanceSqToLine(const QPointF& pt);
        void appendTo(QList<AnimalPair>& ls);
    };

    typedef QList<AnimalPair> PairsList;


private:
    Q_OBJECT

    QVector<Animal*> mAnimals;

    PairsList mInfoPairs;

    int mCollarsCount = 0;
    float mAnimalSize = 0;

    void clear();


public:
    explicit Herd(QObject *parent = nullptr);
    ~Herd();
    void generate(int count, float animalSize, int areaDimeter, int percentageCollars);
    void update(QPointF *attractor,
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
    PairsList infoPairs() { return mInfoPairs; };
    inline int count(){ return mAnimals.count(); }
    int collarsCount(){ return mCollarsCount; }

signals:
};

#endif // HERD_H
