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
               SimTools::HarmonicsGenerator::Params &genParams,
               float genScale,
               int genSmoothIterations,
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

    SimTools::HarmonicsGenerator gen(genParams);

    for( auto h = 0; h < mLawnsDim.height(); h++)  {
        float rowH = h * mLawnDiam - mOffsetH;
        QVector<Lawn*> row;
        row.reserve(mLawnsDim.width());
        for( auto w = 0; w < mLawnsDim.width(); w++) {
            QPointF pos( w * mLawnDiam - mOffsetW, rowH );
            pos += center;
            float capacity = SimTools::clamped( kgPerLawn*0.5 + genScale * gen.sum(pos.x(),pos.y()), 0.0f,  kgPerLawn);
            Lawn* lawn = new Lawn( pos, capacity, kgPerLawn, this );
            mLawns.append(lawn);
            row.append(lawn);
        }
        mLawnsMatrix.append(row);
    }


    // Smooting the result
    for( auto i = 0; i < genSmoothIterations; i++) {
        for( auto h = 0; h < mLawnsDim.height(); h++)  {
            int top = h-1;
            if( top < 0) top = 0;
            int bottom = h + 1;
            if( bottom >= mLawnsDim.height()) bottom = mLawnsDim.height() - 1;

            for( auto w = 0; w < mLawnsDim.width(); w++) {

                int left = w-1;
                if( left < 0) left = 0;
                int right = w + 1;
                if( right >= mLawnsDim.width()) right = mLawnsDim.width() - 1;

                mLawnsMatrix[h][w]->setKg(  (mLawnsMatrix[top][left]->kg() +
                                            mLawnsMatrix[top][w]->kg() +
                                            mLawnsMatrix[top][right]->kg() +
                                            mLawnsMatrix[h][left]->kg() +
                                            mLawnsMatrix[h][right]->kg() +
                                            mLawnsMatrix[bottom][left]->kg() +
                                            mLawnsMatrix[bottom][w]->kg() +
                                           mLawnsMatrix[bottom][right]->kg()) / 8.0f );
            }
        }
    }

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

    float step = tickSeconds * mGrowingSpeed / 60;

    foreach( Lawn* l, mLawns) {
        mKg += l->kg();
        mKgMax += l->kgMax();

        if( mIsGrowing && l->needToGrow() ) {
            l->grow(step);
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

QPointF Meadow::getMapPoint(const QGeoCoordinate &geoPt)
{
    // TODO: implemented it
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
