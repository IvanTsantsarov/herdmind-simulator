#include "bolus.h"
#include "../tools.h"

#ifdef SIMULATION
#include "../../animal.h"
#endif



#ifdef SIMULATION
//////////////////////////////////////////////////////////////
/// Simulation
//////////////////////////////////////////////////////////////

void Bolus::onTimer()
{
    process();
}

bool Bolus::readTemperature()
{
    mT = Tools::rnd(36.5f,36.8f);
    return true;
}

bool Bolus::readMotion()
{
    mAx = Tools::rnd(-100.0f,100.0f);
    mAy = Tools::rnd(-100.0f,100.0f);
    mAz = Tools::rnd(-100.0f,100.0f);
    return true;
}

bool Bolus::sendPackage()
{
    mAnimal->onThisBolusSent(&mPackage);
    return true;
}

#else
//////////////////////////////////////////////////////////////
/// Real conditions
//////////////////////////////////////////////////////////////


#endif // SIMULATION

void Bolus::preparePackage()
{
    mPackage.seq ++;
    mPackage.t = Tools::f2i16(mT, 100.f);
    mPackage.ax = Tools::f2i16(mAx, 1000.f);
    mPackage.ay = Tools::f2i16(mAy, 1000.f);
    mPackage.az = Tools::f2i16(mAz, 1000.f);
}

void Bolus::process()
{
    readTemperature();
    readMotion();
    preparePackage();
    sendPackage();
}
