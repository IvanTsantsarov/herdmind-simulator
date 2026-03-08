#include <bits/stdc++.h>
#include "bolus.h"
#include "accel.h"
#include "../tools.h"

#ifdef SIMULATION
#include "../../animal.h"
#endif



#ifdef SIMULATION
//////////////////////////////////////////////////////////////
/// Simulation
//////////////////////////////////////////////////////////////

Bolus::Bolus(   Animal *animal,
                const QByteArray &devEUI,
                const QByteArray& appKey)
    : LoraDev(QString("%1 Bolus").arg(animal->name()), LoraDev::Profile::Bolus,
              BOLUS_UPDATE_INTERVAL, BOLUS_SEND_INTERVAL,
              devEUI, appKey ), mAnimal(animal)
{
    init();
}

void Bolus::onUpdate()
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



#else
//////////////////////////////////////////////////////////////
/// Real conditions
//////////////////////////////////////////////////////////////
Bolus::Bolus() {
    init();
}

#endif // SIMULATION

void Bolus::init()
{
    mAccel = new Accel();
}

Bolus::~Bolus()
{
    delete mAccel;
}


void Bolus::process()
{
    Protocol::Bolus newPackage; // just set new timestamp
    mPackage = newPackage;
    mT = Tools::f2i16(mT, 100.f);

    readTemperature();
    readMotion();

    mCondition = 0;
    mAccel->addData(mAx, mAy, mAz);

    if( mAccel->getPossibleAtony()) {
        setCondition(Protocol::Bolus::Condition::Atony);
    }

    if( mAccel->getHyperactivity()) {
        setCondition(Protocol::Bolus::Condition::Hyperactivity);
    }

    if( mAccel->getHypomotility()) {
        setCondition(Protocol::Bolus::Condition::Hypomotility);
    }

    if( mT > HIGH_TEMPERATURE ) {
        setCondition(Protocol::Bolus::Condition::HighTemperature);
    }
}

void Bolus::onSend()
{
    sendPackage(mPackage.toByteArray(), sizeof(Protocol::BolusByteArray));
}
