#include "network.h"
#include "animal.h"
#include "herd.h"
#include "hardware/collar/collar.h"
#include "hardware/gateway/gateway.h"
#include "hardware/tools.h"
#include "defines_settings.h"

Network::Network(const QSettings &settings, int gatewaysCount, float areaRadius, QObject *parent)
    : QObject{parent}
{

    if( gatewaysCount < 1 ) {
        gatewaysCount = 1;
    }
    auto genPos = [&](Gateway* gw) {
        gw->mPos.setX( Tools::rnd(0, areaRadius) );
        gw->mPos.setY( Tools::rnd(0, areaRadius) );
    };

    mGateways.reserve(gatewaysCount);

    mEdge = new Gateway(settings);
    genPos( mEdge );
    mGateways.append( mEdge );

    for( auto i = 1; i < gatewaysCount; i++) {
        Gateway* gw = new Gateway(settings);
        genPos( gw );
        mGateways.append( gw );
    }

}

Network::~Network()
{
    foreach( Gateway* gw, mGateways) {
        delete gw;
    }
}



void Network::update(Herd *herd, int maxCollarGatewayDistance)
{

    mPairs.clear();

    int collarDistSq = maxCollarGatewayDistance * maxCollarGatewayDistance;

    auto makePair = [&](Gateway* gw, Animal* a) {
        if( !a->hasCollar() ) {
            return;
        }

        GatewayPair pair( gw,  a);
        if( pair.checkTransmitVisibility(collarDistSq)) {
            pair.appendTo(mPairs);
        }
    };


    foreach(Gateway* gw, mGateways ) {
        for( int i = 0; i < herd->animalsCount(); i ++ ) {
            makePair(gw, herd->animal(i));
        }
    }

}

Network::GatewayPair::GatewayPair(Gateway *gateway, Gateway *gatewayOther) : mGateway(gateway), mGatewayOther(gatewayOther)
{

}

Network::GatewayPair::GatewayPair(Gateway *gateway, Animal *collarAnimal) : mGateway(gateway), mCollarAnimal(collarAnimal)
{

}

void Network::GatewayPair::appendTo(QList<GatewayPair> &ls)
{
    ls.append(*this);
}

float Network::GatewayPair::distanceSq()
{
    QPointF d;

    if( mGatewayOther ) {
        d = mGateway->mPos - mGatewayOther->mPos;
    } else {
        d = mGateway->mPos - mCollarAnimal->pt();
    }

    return d.x()*d.x() + d.y()*d.y();
}

QPointF Network::GatewayPair::firstPt()
{
    return mGateway->mPos;
}


QPointF Network::GatewayPair::secondPt()
{
    return mCollarAnimal ? mCollarAnimal->pt() : mGatewayOther->mPos;
}

bool Network::GatewayPair::isSending()
{
    return mCollarAnimal ? mCollarAnimal->collar()->isSending() : mGatewayOther->isSending();
}
