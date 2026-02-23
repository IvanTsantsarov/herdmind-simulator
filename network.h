#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include <QList>
#include <QLineF>

class Gateway;
class Animal;
class Herd;

class Network : public QObject
{
    Q_OBJECT

    Gateway* mEdge = nullptr;

public:
    explicit Network(int gatewaysCount, float areaRadius, QObject *parent = nullptr);
    ~Network();


    inline int gatewaysCount(){ return mGateways.count(); }

    class GatewayPair {
        Gateway* mGateway = nullptr;
        Animal* mCollarAnimal  = nullptr;
        Gateway* mGatewayOther = nullptr;
        QLineF mLine;
    public:
        GatewayPair( Gateway* gateway, Gateway* gatewayOther );
        GatewayPair( Gateway* gateway, Animal* collarAnimal );

        inline Animal* collarAnimal(){ return mCollarAnimal;};
        inline Gateway* gateway() { return mGateway; }
        inline Gateway* gatewayOther() { return mGatewayOther; }
        float distanceSqToLine(const QPointF& pt);
        void appendTo(QList<GatewayPair>& ls);
        float distanceSq();
        inline bool hasAnimal() { return nullptr != mCollarAnimal; }
        QPointF firstPt();
        QPointF secondPt();
        bool isSending();
        bool checkTransmitVisibility(float maxDistanceSq) { return distanceSq() < maxDistanceSq*maxDistanceSq; }
    };

    typedef QVector<GatewayPair> PairsListG;

    PairsListG& pairs() { return mPairs; };

    Gateway* gateway(int index){ return mGateways[index]; }
    inline Gateway* edge(){ return mEdge; }

    void update(Herd* herd, int maxCollarGatewayDistance);

private:

    QList <Gateway*> mGateways;

    PairsListG mPairs;

signals:
};

#endif // NETWORK_H
