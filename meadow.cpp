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

    float qttyPerLawn = kgPerSqMeter / lawnsCount();
    float offsetW = mAreaSize.width() * 0.5f;
    float offsetH = mAreaSize.height() * 0.5f;

    mLawns.reserve(lawnsCount());
    float d = lawnDiam();

    for( auto h = 0; h < mLawnsDim.height(); h++)  {
        float rowH = h*d - offsetH;
        for( auto w = 0; w < mLawnsDim.width(); w++) {
            Lawn* lawn = new Lawn( QPointF(w*d - offsetW, rowH ), qttyPerLawn, this );
            mLawns.append(lawn);
        }
    }
}

Meadow::Lawn *Meadow::closestAvailable(const QPointF &pos, const Lawn* current)
{
    Lawn* closest = nullptr;
    float minDistSq = static_cast<float>(INT_MAX);
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

Meadow::Lawn::Lawn(const QPointF &position, float startingQtty, Meadow *parent) :
    mPos(position), mQttyStart(startingQtty), mMeadow(parent) {

    mAnimals.reserve(mMeadow->animalsPerLawn());
}

bool Meadow::Lawn::graze(float q) {
    if( !isDepleted() ) {
        return false;
    }

    mQtty -= q;
    return isDepleted();
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


