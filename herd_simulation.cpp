#include "herd.h"
#include "hardware/bolus/bolus.h"
//#include "simtools.h"
#include "shepherd.h"
//#include "hardware/tools.h"
#include "animal.h"
//#include "defines.h"

void Herd::updateSimulation(  float tickSeconds,
                            Meadow* meadow,
                            QPointF* attractor,
                            bool isCorrectCollision,
                            float attractorPower,
                            float attractionDistance,
                            float repellingDistance,
                            float collidingDistance,
                            float maxSpeed,
                            float friction,
                            float rotationFading,
                            float maxTransmitDistance,
                            float maxTransmitAngle )
{
    Q_UNUSED(maxSpeed);

    mMeadow = meadow;

    float minTransmitAngleCos = cosf(maxTransmitAngle);

    mPairsBC.clear();

    bool mustRun = mIsEnabledHerding && (attractor || mIsShepherdActive);

    foreach(Animal* animal, mAnimals) {
        animal->run( mustRun );
    }


    if( !mustRun ) {
        ////////////////////////////////////////////////////////////////////
        /// Grazing
        ////////////////////////////////////////////////////////////////////

        if( !attractor && !mIsShepherdActive) {

            foreach(Animal* animal, mAnimals) {
                if( !animal->lawn() && !animal->isMoving() && !animal->isResting() ) {
                    Meadow::Lawn* lawn = meadow->bestAvailable(animal->pt());
                    if( lawn ) {
                        animal->walkTo(QVector2D(lawn->pos()));
                        animal->attach(lawn);
                    }
                }

                if( animal->lawn() && !animal->isGrazing() ) {
                    if( animal->isArrived() ) {
                        if( !animal->graze() ) {
                            animal->dettach();
                        }
                    }else
                        // if it's not arrived
                        if( !animal->isMoving()) {
                            animal->dettach();
                        }
                }

                animal->updateBehavior(tickSeconds, friction, rotationFading);
            }
        }
    }else
    ////////////////////////////////////////////////////////////////////
    /// Running
    ////////////////////////////////////////////////////////////////////
    {
        QVector2D shepherdPosition;
        if( mIsShepherdActive ) {
            QPointF pt = mShepherd->step();
            shepherdPosition = QVector2D(pt);
        }

        // process attractor force if available
        foreach(Animal* animal, mAnimals) {
            if( animal->lawn() ) {
                animal->dettach();
            }

            if( attractor ) {
                animal->updateRunning(QVector2D(*attractor), attractorPower,
                                      attractionDistance, repellingDistance, friction);
            }

            if( mIsShepherdActive ) {
                animal->updateRunning(shepherdPosition, attractorPower,
                                      attractionDistance, repellingDistance, friction);
            }

        }
    }

    // update position and bolus
    foreach(Animal* animal, mAnimals) {
        animal->updateCommon(tickSeconds);
    }

    // Precision collision is disabled - too expensinve
    if( isCorrectCollision ) {
        // process collision
        int collisionCounter = 0;
        while( processCollision(collidingDistance) ) {
            if( ++collisionCounter > mAnimals.count() ) {
                break;
            };
        }
    }else {
        processCollision(collidingDistance);
    }


    float maxTransmitDistanceSq = maxTransmitDistance*maxTransmitDistance;

    // clear statistics lists
    foreach(Animal* animal, mAnimals) {
        animal->clearObservers();
        animal->clearObserving();
    }

    // update boluses visibilities for all animals with collars
    foreach(Animal* animal, mCollars) {

        foreach(Animal* otherAnimal, mAnimals) {

            if( !otherAnimal->hasBolus() ) {
                continue;
            }

            float distanceOtherSq = animal->distanceSq(otherAnimal);

            if( distanceOtherSq > maxTransmitDistanceSq ) {
                continue;
            }

            if( !animal->isAhead(otherAnimal, minTransmitAngleCos ) ) {
                continue;
            }

            // add it to pairs if directly visible
            AnimalPair ap(animal, otherAnimal);
            //if(  checkTransmitVisibility(ap, distanceOtherSq, minTransmitAngleCos) ) {
            ap.appendTo( mPairsBC );
            //}

        }
    }

}

void Herd::updateReal()
{

}

