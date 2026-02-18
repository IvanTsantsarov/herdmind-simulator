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
                const QByteArray &joinEUI,
                const QByteArray& appKey)
    : LoraDev("Bolus", LoraDev::Profile::Bolus,
              BOLUS_UPDATE_INTERVAL, BOLUS_SEND_INTERVAL,
              devEUI, joinEUI, appKey ), mAnimal(animal)
{
    init();
    mPackage.b = 100;
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


void Bolus::preparePackage()
{
    mT = Tools::f2i16(mT, 100.f);
    mPackage.c = 0;
}

void Bolus::process()
{
    readTemperature();
    readMotion();

    mCondition = 0;
    mAccel->addData(mAx, mAy, mAz);

    if( mAccel->getPossibleAtony()) {
        setCondition(Condition::Atony);
    }

    if( mAccel->getHyperactivity()) {
        setCondition(Condition::Hyperactivity);
    }

    if( mAccel->getHypomotility()) {
        setCondition(Condition::Hypomotility);
    }

    if( mT > HIGH_TEMPERATURE ) {
        setCondition(Condition::HighTemperature);
    }

    mPackage.s++;
}

void Bolus::onSend()
{
    sendPackage(&mPackage, sizeof(mPackage));
}
