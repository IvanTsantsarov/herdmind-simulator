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
    QSize mLawnsDim;
    uint mAnimalsPerLawn;

public:
    explicit Meadow(const QPointF &center,
                    const QSize& areaSize,
                    float lawnRad,
                    float kgPerSqMeter,
                    uint animalsPerLawn,
                    QObject *parent = nullptr);

    class Lawn {
        QPointF mPos;
        float mQttyStart, mQtty;
        Meadow* mMeadow;
        QList<Animal*> mAnimals;
    public:

        Lawn(const QPointF& position, float startingQtty, Meadow* parent);
        inline QPointF pos(){ return mPos; }

        inline bool isDepleted(){ return mQtty >= 0; }
        bool hasSpace(){ return mAnimals.count() < mMeadow->animalsPerLawn(); }

        bool graze(float q);
        float distSq(const QPointF &pt);

        inline float qtty(){ return mQtty; };
        qint8 qttyPercents(){ return mQtty/mQttyStart * 100; };
        qint8 qtty255(){ return mQtty/mQttyStart * 255; };
        void attach(Animal* animal){ mAnimals.append(animal);        }
        void deattach(Animal* animal);

    };

    inline float lawnRadius(){ return mLawnRadius; }
    inline float lawnDiam(){ return mLawnRadius * 2.0f; }
    inline int lawnsCount(){ return mLawnsDim.width() * mLawnsDim.height(); }
    Lawn* closestAvailable(const QPointF& pos, const Lawn* current = nullptr);
    inline uint animalsPerLawn(){return mAnimalsPerLawn; }

protected:
    QVector<Lawn*> mLawns;

public:
    inline QVector<Lawn*> & lawns(){ return mLawns; }

signals:
};

#endif // MEADOW_H
