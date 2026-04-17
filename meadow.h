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

    QGeoCoordinate mGeoCenter;

    bool mIsGrowing = false;
    float mGrowingSpeed;

public:
    explicit Meadow(const QPointF &center,
                    const QGeoCoordinate& geoCenter,
                    const QSize& dimension,
                    float lawnRad,
                    float kgPerSqMeter,
                    float growingSpeed,
                    uint animalsPerLawn,
                    QObject *parent = nullptr);

    ~Meadow();

    class Lawn {
        QPointF mPos;
        QGeoCoordinate mCoord;
        float mKgMax = 1.0f, mKg = 0.0f, mKgStart = 0.0f;
        Meadow* mMeadow;
        QList<Animal*> mAnimals;

    public:

        bool mustUpdate = true;

        Lawn(const QPointF& position, float currentKg, float maxKg, Meadow* parent);
        inline QPointF pos(){ return mPos; }

        inline bool isDepleted(){ return mKg <= 0.01; }
        inline bool hasSpace(){ return mAnimals.count() < mMeadow->animalsPerLawn(); }
        inline int animalsCount(){ return mAnimals.count(); }
        inline void refill(){ mKg = mKgStart; mustUpdate = true; }

        bool graze(float weight);
        bool grow(float weight);
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

    void setGrowing(bool isGrowing) { mIsGrowing = isGrowing; }
    inline bool isGrowing() { return mIsGrowing; }
    inline float lawnRadius(){ return mLawnRadius; }
    inline float lawnDiam(){ return mLawnDiam; }
    inline int lawnsCount(){ return mLawns.count(); }
    Lawn* closestAvailable(const QPointF& pos, const Lawn* current = nullptr);
    Lawn* bestAvailable(const QPointF& pos, const Lawn* current = nullptr);
    Lawn* byPos(float x, float y);
    inline Lawn* byPos( const QPointF& pos){ return byPos(pos.x(), pos.y()); }

    inline uint animalsPerLawn(){return mAnimalsPerLawn; }
    inline float kgMax(){ return mKgMax; }
    inline float kg(){ return mKg; }
    inline float kgRatio(float multiplyBy = 1.0f){ return multiplyBy * mKg / mKgMax; }

    void update(float tickSeconds);
    void refill();


protected:
    QVector<QVector<Lawn*>> mLawnsMatrix;
    QVector<Lawn*> mLawns;

public:
    inline QVector<Lawn*> & lawns(){ return mLawns; }
    inline QVector<QVector<Lawn*>> & lawnsMatrix(){ return mLawnsMatrix; }
    QGeoCoordinate getGeoLocation(const QPointF& mapPos);
    Lawn* byIndex( int lw, int lh );
    inline QSize dim(){ return mLawnsDim; }


signals:
};

#endif // MEADOW_H
