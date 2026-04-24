#ifndef VIRTUALFENCE_H
#define VIRTUALFENCE_H

#include <QObject>
#include <QPolygonF>
#include <QGeoCoordinate>


class Meadow;
class Scene;

class VirtualFence : public QPolygonF
{
    Q_OBJECT

    Meadow* mMeadow = nullptr;

public:
    explicit VirtualFence(Meadow* meadow);
    QGeoCoordinate geoPoint(int index);

    signals:
};

#endif // VIRTUALFENCE_H
