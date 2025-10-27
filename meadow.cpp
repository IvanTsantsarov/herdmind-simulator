#include "meadow.h"

Meadow::Meadow(const QPointF& center,
               const QSize &areaSize,
               float lawnR,
               float kgPerSqMeter,
               uint animalsPerLawn,
               QObject *parent)
    : QObject(parent), mAreaSize(areaSize), mLawnRadius(lawnR), mAnimalsPerLawn(animalsPerLawn)
{

    mLawnsDim = QSize( mAreaSize.width() / lawnDiam(),
                      mAreaSize.height() / lawnDiam() );

    float kgPerLawn = kgPerSqMeter * (lawnR * lawnR * 4);
    float offsetW = mAreaSize.width() * 0.5f;
    float offsetH = mAreaSize.height() * 0.5f;

    mLawns.reserve(mLawnsDim.width() * mLawnsDim.height());
    float d = lawnDiam();

    // mLawnsMatrix.reserve(mLawnsDim.height());

    for( auto h = 0; h < mLawnsDim.height(); h++)  {
        float rowH = h*d - offsetH;
        // QVector<Lawn*> row;
        // row.reserve(mLawnsDim.width());
        for( auto w = 0; w < mLawnsDim.width(); w++) {
            Lawn* lawn = new Lawn( QPointF(w*d - offsetW, rowH ), kgPerLawn, this );
            mLawns.append(lawn);
            // row.append(lawn);
        }
        // mLawnsMatrix.append(row);
    }
}

Meadow::Lawn *Meadow::closestAvailable(const QPointF &pos, const Lawn* current)
{
    Lawn* closest = nullptr;
    float minDistSq = static_cast<float>(mAreaSize.width()+mAreaSize.height());
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

Meadow::Lawn::Lawn(const QPointF &position, float startingKg, Meadow *parent) :
    mPos(position), mKgStart(startingKg), mKg(startingKg), mMeadow(parent) {

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

float Meadow::Lawn::distSq(const QPointF &pt)
{
    float dx = mPos.x() - pt.x();
    float dy = mPos.y() - pt.y();
    return dx*dx + dy*dy;
}

void Meadow::Lawn::deattach(Animal *animal)
{
    int index = mAnimals.indexOf(animal);
    mAnimals.removeAt(index);
}


