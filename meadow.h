#ifndef MEADOW_H
#define MEADOW_H

#include <QSize>
#include <QPointF>
#include <QObject>
#include <QGeoCoordinate>


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
    float mFood = 0.0f; // 0..1
    float mKg = 0.0f;
    float mKgMax = 0.0f;

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
        QGeoCoordinate mCoord;
        float mKgMax = 1.0f, mKg = 0.0f;
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
        inline float kgNorm(){ return mKg/mKgMax; };
        int kgPercents(){ return kgNorm() * 100; };
        inline float kgMax(){ return mKgMax; }
        inline QGeoCoordinate& coord(){ return mCoord; }

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
    inline Lawn *lawn( const QPointF& pos){ return lawn(pos.x(), pos.y()); }

    inline uint animalsPerLawn(){return mAnimalsPerLawn; }
    inline float kgMax(){ return mKgMax; }
    inline float kg(){ return mKg; }
    inline float kgRatio(float multiplyBy = 1.0f){ return multiplyBy * mKg / mKgMax; }

    void update();


protected:
    QVector<QVector<Lawn*>> mLawnsMatrix;
    QVector<Lawn*> mLawns;

public:
    inline QVector<Lawn*> & lawns(){ return mLawns; }
    inline QVector<QVector<Lawn*>> & lawnsMatrix(){ return mLawnsMatrix; }


signals:
};

#endif // MEADOW_H
