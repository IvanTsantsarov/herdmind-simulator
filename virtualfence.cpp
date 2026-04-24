#include "virtualfence.h"
#include "meadow.h"
#include "scene.h"



VirtualFence::VirtualFence(Meadow* meadow)
    : mMeadow{meadow}
{

}


QGeoCoordinate VirtualFence::geoPoint(int index)
{
    return mMeadow->getGeoLocation(point(index));
}



