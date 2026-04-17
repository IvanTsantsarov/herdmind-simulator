#include "meadow.h"
#include "defines.h"
#include "simtools.h"

Meadow::Meadow(const QPointF& center,
               const QGeoCoordinate &geoCenter,
               const QSize &dimension,
               float lawnR,
               float kgPerSqMeter,
               float growingSpeed,
               uint animalsPerLawn,
               QObject *parent)
    : QObject(parent), mLawnRadius(lawnR),
    mLawnDiam(2.0f*lawnR),
    mLawnsDim(dimension),
    mAnimalsPerLawn(animalsPerLawn),
    mGrowingSpeed(growingSpeed)
{
    mGeoCenter = geoCenter;

    mAreaSize = QSize( mLawnsDim.width() * mLawnDiam ,
                       mLawnsDim.height() * mLawnDiam );

    float kgPerLawn = kgPerSqMeter * (lawnR * lawnR * 4);
    mOffsetW = mAreaSize.width() * 0.5f;
    mOffsetH = mAreaSize.height() * 0.5f;

    int count = mLawnsDim.width() * mLawnsDim.height();
    mLawns.reserve(count);
    mLawnsMatrix.reserve(mLawnsDim.height());

    int genPolyCount = 10;
    float genPolyMul = 1.0f / (float) genPolyCount;

    float radius = sqrtf( dimension.width()*dimension.width() + dimension.height()*dimension.height()) * 0.5f;
    SimTools::HarmonicsGenerator gen(radius * 10, genPolyCount, kgPerLawn, 4*kgPerLawn, ANIMAL_LENGTH*10, ANIMAL_LENGTH*100);

    for( auto h = 0; h < mLawnsDim.height(); h++)  {
        float rowH = h * mLawnDiam - mOffsetH;
        QVector<Lawn*> row;
        row.reserve(mLawnsDim.width());
        for( auto w = 0; w < mLawnsDim.width(); w++) {
            QPointF pos( w * mLawnDiam - mOffsetW, rowH );
            pos += center;
            float capacity = SimTools::clamped( kgPerLawn*0.5 + genPolyMul * gen.sum(pos.x(),pos.y()), 0.0f,  kgPerLawn);
            Lawn* lawn = new Lawn( pos, capacity, kgPerLawn, this );
            mLawns.append(lawn);
            row.append(lawn);
        }
        mLawnsMatrix.append(row);
    }

    // Commented code of smooting makes the
    // whole simulator slower whithout even executing itself!
    // WTF ?!?!
 /*
    for( auto i = 0; i < 20; i++) {
        for( auto h = 1; h < mLawnsDim.height()-1; h++)  {
            for( auto w = 1; w < mLawnsDim.width()-1; w++) {
                mLawnsMatrix[h][w]->setKg(  (mLawnsMatrix[h-1][w-1]->kg() +
                                            mLawnsMatrix[h-1][w]->kg() +
                                            mLawnsMatrix[h-1][w+1]->kg() +
                                            mLawnsMatrix[h][w-1]->kg() +
                                            mLawnsMatrix[h][w+1]->kg() +
                                            mLawnsMatrix[h+1][w-1]->kg() +
                                            mLawnsMatrix[h+1][w]->kg() +
                                           mLawnsMatrix[h+1][w+1]->kg()) / 8.0f );
            }
        }
    }
 */
}

Meadow::~Meadow()
{
    foreach( Lawn* l, mLawns) {
        delete l;
    }
}

void Meadow::update(float tickSeconds)
{
    mKgMax = mKg = 0.0f;

    foreach( Lawn* l, mLawns) {
        mKg += l->kg();
        mKgMax += l->kgMax();

        if( mIsGrowing ) {
            l->grow(tickSeconds * mGrowingSpeed / 60);
        }
    }

}

void Meadow::refill()
{
    foreach( Lawn* l, mLawns) {
        l->refill();
    }
}



Meadow::Lawn *Meadow::byPos(float x, float y)
{
    int lw = (x + mOffsetW + mLawnRadius) / mLawnDiam;
    int lh = (y + mOffsetH + mLawnRadius) / mLawnDiam;

    return byIndex(lh, lw);
}

Meadow::Lawn *Meadow::closestAvailable(const QPointF &pos, const Lawn* current)
{
    Lawn* closest = nullptr;
    float minDistSq = static_cast<float>(mAreaSize.width()*mAreaSize.height());
    for(auto i = 0; i < mLawns.count(); i ++) {
        Lawn* lawn = mLawns[i];

        if( lawn->isDepleted() ) {
            continue;
        }

        if( !lawn->hasSpace() ) {
            continue;
        }

        if( current && current == lawn) {
            continue;
        };

        float distSq = lawn->distSq(pos);
        if( distSq < minDistSq ) {
            closest = lawn;
            minDistSq = distSq;
        }
    }

    return closest;
}

Meadow::Lawn *Meadow::bestAvailable(const QPointF &pos, const Lawn *current)
{
    Lawn* closest = nullptr;
    float bestRatio = 0;
    for(auto i = 0; i < mLawns.count(); i ++) {
        Lawn* lawn = mLawns[i];

        if( lawn->isDepleted() ) {
            continue;
        }

        if( !lawn->hasSpace() ) {
            continue;
        }

        if( current && current == lawn) {
            continue;
        };

        float ratio = lawn->kg()*lawn->kg() / lawn->distSq(pos);
        if( ratio > bestRatio ) {
            closest = lawn;
            bestRatio = ratio;
        }
    }

    return closest;
}


Meadow::Lawn::Lawn(const QPointF &position, float currentKg, float maxKg, Meadow *parent) :
    mPos(position), mKgMax(maxKg), mKg(currentKg), mKgStart(currentKg), mMeadow(parent) {

    mAnimals.reserve(mMeadow->animalsPerLawn());
}

bool Meadow::Lawn::graze(float weight) {

    if( isDepleted() ) {
        return true;
    }

    mKg -= weight;

    if( isDepleted() ) {
        if( mKg < 0.0f) {
            mKg = 0.0f;
        }
        return true;
    }

    return false;
}

bool Meadow::Lawn::grow(float weight)
{
    mKg += weight;
    if( mKg > mKgStart ) {
        mKg = mKgStart;
        return true;
    }

    mustUpdate = true;
    return false;
}

float Meadow::Lawn::distSq(const QPointF &pt)
{
    float dx = mPos.x() - pt.x();
    float dy = mPos.y() - pt.y();
    return dx*dx + dy*dy;
}

void Meadow::Lawn::dettach(Animal *animal)
{
    int index = mAnimals.indexOf(animal);
    mAnimals.removeAt(index);
}


QGeoCoordinate Meadow::getGeoLocation(const QPointF &mapPos)
{
    const double metersPerDegLat = 111320.0;

    double lat = mGeoCenter.latitude();
    double lon = mGeoCenter.longitude();

    double dLat = mapPos.y() / metersPerDegLat;

    double metersPerDegLon = metersPerDegLat * qCos(qDegreesToRadians(lat));
    double dLon = mapPos.x() / metersPerDegLon;

    return QGeoCoordinate(lat + dLat, lon + dLon);
}

Meadow::Lawn *Meadow::byIndex(int lw, int lh)
{
    if( lw < 0 || lw >= mLawnsDim.width() ) {
        return nullptr;
    }

    if( lh < 0 || lh >= mLawnsDim.height() ) {
        return nullptr;
    }

    return mLawnsMatrix[lh][lw];
}
