#ifndef MEADOW_H
#define MEADOW_H

#include <QSize>
#include <QPointF>
#include <QObject>


class Animal;

class Meadow : public QObject
{
    Q_OBJECT

    QSize mAreaSize;
    float mLawnRadius;
    float mLawnDiam;
    QSize mLawnsDim;
    uint mAnimalsPerLawn;
    float mOffsetW, mOffsetH;

public:
    explicit Meadow(const QPointF &center,
                    const QSize& areaSize,
                    float lawnRad,
                    float kgPerSqMeter,
                    uint animalsPerLawn,
                    QObject *parent = nullptr);

    ~Meadow();

    class Lawn {
        QPointF mPos;
        float mKgStart, mKg;
        Meadow* mMeadow;
        QList<Animal*> mAnimals;
    public:

        Lawn(const QPointF& position, float currentKg, float maxKg, Meadow* parent);
        inline QPointF pos(){ return mPos; }

        inline bool isDepleted(){ return mKg <= 0.01; }
        inline bool hasSpace(){ return mAnimals.count() < mMeadow->animalsPerLawn(); }
        inline int animalsCount(){ return mAnimals.count(); }

        bool graze(float weight);
        float distSq(const QPointF &pt);

        inline float kg(){ return mKg; };
        inline float kgNorm(){ return mKg/mKgStart; };
        int kgPercents(){ return kgNorm() * 100; };

        // quint8 kg255(){ return mKg/mKgStart * 255; };
        void attach(Animal* animal){ mAnimals.append(animal);        }
        void dettach(Animal* animal);

    };

    inline float lawnRadius(){ return mLawnRadius; }
    inline float lawnDiam(){ return mLawnDiam; }
    inline int lawnsCount(){ return mLawns.count(); }
    Lawn* closestAvailable(const QPointF& pos, const Lawn* current = nullptr);
    Lawn* bestAvailable(const QPointF& pos, const Lawn* current = nullptr);
    Lawn* lawn(float x, float y);

    inline uint animalsPerLawn(){return mAnimalsPerLawn; }


protected:
    QVector<QVector<Lawn*>> mLawnsMatrix;
    QVector<Lawn*> mLawns;

public:
    inline QVector<Lawn*> & lawns(){ return mLawns; }
    inline QVector<QVector<Lawn*>> & lawnsMatrix(){ return mLawnsMatrix; }


signals:
};

#endif // MEADOW_H
