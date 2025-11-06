#include "network.h"
#include "animal.h"
#include "herd.h"
#include "hardware/collar/collar.h"
#include "hardware/gateway/gateway.h"
#include "hardware/tools.h"

Network::Network(int gatewaysCount, float areaRadius, QObject *parent)
    : QObject{parent}
{
    mGateways.reserve(gatewaysCount);
    for( auto i = 0; i < gatewaysCount; i++) {
        Gateway* gw = new Gateway;
        gw->mPos.setX( Tools::rnd(0, areaRadius) );
        gw->mPos.setY( Tools::rnd(0, areaRadius) );
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


    foreach(Gateway* gw, mGateways ) {

        for( int i = 0; i < herd->animalsCount(); i ++ ) {
            Animal* a = herd->animal(i);
            if( !a->hasCollar() ) {
                continue;
            }

            GatewayPair pair( gw,  a);
            if( pair.checkTransmitVisibility(collarDistSq)) {
                pair.appendTo(mPairs);
            }
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
    return mCollarAnimal ? mCollarAnimal->collar()->isSendingSimulation() : mGatewayOther->isSendingSimulation();
}
