#ifndef ANIMAL_H
#define ANIMAL_H

#include <QVector2D>
#include <QList>
#include "meadow.h"
#include "hardware/loradev.h"


class Herd;
class Bolus;
class Collar;


#define FEMALE_NAMES_COUNT 3181
#define MALE_NAMES_COUNT 41

class Animal
{
    static const QString mFemaleNames[FEMALE_NAMES_COUNT];
    static const QString mMaleNames[MALE_NAMES_COUNT];

    QString mName;
    bool mIsMale = false;

    enum struct State {
        sitting = 0,
        resting = 1,
        grazing = 2,
        going = 3,
        stopping = 4,
        avoiding = 5,
        running = 6
    };


    State mState = State::sitting;
    State mStatePrev = State::sitting;
    quint64 mStateMsec = 0;

    static QString stateString(Animal::State state) {
        switch(state) {
        case State::sitting: return QString("Sitting");
        case State::resting: return QString("Resting");
        case State::grazing: return QString("Grazing");
        case State::going: return QString("Going");
        case State::stopping: return QString("Stopping");
        case State::avoiding: return QString("Avoiding");
        case State::running: return QString("Running");
        }

        return "Unknown!!!";
    }

    inline QString currentStateString(){ return Animal::stateString(mState); }

    QVector2D mDestination;
    float mArrivingDistance;
    Animal* mObstacle = nullptr;
    float mStamina = 1.0f;

public:

    inline bool isSitting(){ return State::sitting == mState; }
    inline bool isResting(){ return State::resting == mState; }
    inline bool isGrazing(){ return State::grazing == mState; }
    inline bool isGoing(){ return State::going == mState; }
    inline bool isStopping(){ return State::stopping == mState; }
    inline bool isAvoiding(){ return State::avoiding == mState; }
    inline bool isRunning(){ return State::running == mState; }


    bool isMoving(){ return mState >= State::going && mState <= State::running; }

    inline bool wasGrazing(){ return State::grazing == mStatePrev; }
    inline bool wasSitting(){ return State::sitting == mStatePrev; }
    inline bool wasGoing(){ return State::going == mStatePrev; }
    inline bool wasStopping(){ return State::stopping == mStatePrev; }
    inline bool wasAvoiding(){ return State::avoiding == mStatePrev; }
    inline bool wasRunning(){ return State::running == mState; }

    static QList<int> namesIndices(bool isMale);

private:

    friend class Bolus;
    friend class Collar;

    Herd* mHerd = nullptr;
    Bolus* mBolus = nullptr;
    Collar* mCollar = nullptr;

    QVector2D mPosition;
    QVector2D mVelocity;
    QVector2D mDirection;
    float mSpeed = 0.0f;
    float mRotationAngle = 0.0f;
    float mRotationAngleTarget = 0.0f;

    void updateRotationAngleTarget();
    void updateSpeedAndDirection();

    // count of the animals with collars that sees this animal
    QList<Animal*> mObservers;
    QList<Animal*> mObserving;

    Meadow::Lawn* mLawn = nullptr;

    // how many readings from other boluses
    uint mReadings = 0;

    float mGrazingCapacity; // in kilograms per 24 hours

    void setState( State newState );
    quint64 stateMsec();
    int stateSec() { return stateMsec() / 1000; }

    void constructAnimal(Herd* herd, bool isMale, const QString& name, float x, float y, float grazingCapacity );
public:

    Animal(Herd* herd, bool isMale, int nameIndex, float x, float y, float grazingCapacity );
    Animal(Herd* herd, bool isMale, const QString& name, float x, float y, float grazingCapacity );
    ~Animal();

    inline QVector2D& p(){ return mPosition; }
    inline QPointF pt(){ return QPointF(mPosition.x(), mPosition.y()); }
    inline QVector2D& v(){ return mVelocity; }
    float distanceSq(Animal* other);
    void updateRunning(const QVector2D &p,
               float attractionPower,
               float attractionDistance,
               float repellingDistance, float friction);

    inline Bolus* bolus(){ return mBolus; }
    inline Collar* collar(){ return mCollar; }

    void setPos(float x, float y){ mPosition.setX(x), mPosition.setY(y); }
    void run(bool is = true);

    bool goTo(const QVector2D& destination, float speed, float arrivingDistance);
    bool walkTo(const QVector2D& destination);
    void lookAt(const QVector2D& destination);

    // return true if info from bolus is sent to the other
    bool collide(Animal* other, float minCollideDistance);
    void updateBehavior(float tickSeconds, float friction, float rotationFading);
    void updateCommon(float tickSeconds);
    inline float rotationAngle(){ return mRotationAngle; }
    inline float rotationAngleTarget(){ return mRotationAngleTarget; }

    void putBolus(const QByteArray &devEUI = QByteArray(),
                  const QByteArray &joinEUI = QByteArray(),
                  const QByteArray& appKey = QByteArray());

    void putCollar(const QByteArray &devEUI = QByteArray(),
                   const QByteArray &joinEUI = QByteArray(),
                   const QByteArray& appKey = QByteArray());

    bool hasBolus() const { return nullptr != mBolus; }
    bool hasCollar() const { return nullptr != mCollar; }

    bool isSideVisible(Animal *a, float maxCosAngle);
    bool isAhead(Animal* animal, float maxCosAngle );

    void clearObservers(){ mObservers.clear(); }
    void addObserver(Animal* a){ mObservers.append(a); }
    int observersCount(){ return mObservers.count(); }

    void clearObserving(){ mObserving.clear(); }
    void addObserving(Animal* a){ mObserving.append(a); }
    int observingCount(){ return mObserving.count(); }

    uint readings(){ return mReadings; }

    void kick();
    bool isCloseToDestination();
    bool isArrived();


    void attach(Meadow::Lawn *newLawn);
    inline void dettach(){ attach(nullptr); };
    inline Meadow::Lawn* lawn(){ return mLawn; }

    // return false if lawn depleted
    bool graze();

    QString info();
    QString jsonInfo(bool isDevicesList);
};

#endif // ANIMAL_H
