#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include "herd.h"
#include "hardware/bolus/bolus.h"
#include "simtools.h"
#include "shepherd.h"
#include "hardware/tools.h"
#include "animal.h"
#include "defines.h"

#define ANIMAL_MIN_DISTANCE 0.2

QList<LoraDev *> Herd::gatherDevices()
{
    QList<LoraDev*> ls;

    foreach(Animal* a, mAnimals) {
        if( a->hasBolus() ) {
            ls.append(a->bolus());

        }
        if( a->hasCollar() ) {
            ls.append(a->collar());
        }
    }

    return ls;
}

void Herd::clear()
{
    foreach(Animal* a, mAnimals) {
        delete a;
    }
    mAnimals.clear();
    mCollars.clear();
    mPairsBC.clear();
}

Herd::Herd(bool isSim, QObject *parent)
    : QObject{parent}, mIsSimulation(isSim)
{
    // mIsEnabledGrazing = false; // trash
    // mIsEnabledHerding = false; // trash
}

Herd::~Herd()
{
    if( mShepherd ) {
        delete mShepherd;
        mShepherd = nullptr;
    }
    clear();
}


bool Herd::load(const QString &jsonPath, int areaDimeter, float animalSize, float grazingCapacity )
{
    mIsSimulation = true;

    qDebug() << "Loading herd from" << jsonPath;

    float areaRadius = beforeGeneration( areaDimeter, animalSize);

    bool isOK;
    QByteArray content = gSimTools->fileRead(jsonPath, &isOK);
    if( !isOK ) {
        qWarning() << "Error reading file:" << jsonPath;
        return false;
    }

    QJsonParseError err;
    QJsonArray jarr = QJsonDocument::fromJson(content, &err).array();
    if( QJsonParseError::NoError != err.error ) {
        qCritical() << "Herd::load error:" << err.errorString() << err.offset << jsonPath;
        QString jsonListStr = QString::fromUtf8(content);
        qDebug() << jsonListStr;
        return false;
    }

    mAnimals.reserve(jarr.size());
    for(const auto& jsonElement: jarr) {
        QJsonObject jobj = jsonElement.toObject();

        float x = Tools::rnd(-areaRadius, areaRadius);
        float y = Tools::rnd(-areaRadius, areaRadius);

        Animal* animal = new Animal(this, jobj["male"].toBool(), jobj["name"].toString(), x, y, grazingCapacity );

        if( jobj.contains("bolus") ) {
            animal->putBolus()->setFromJson(jobj["bolus"].toObject());
        };


        if( jobj.contains("collar") ) {
            animal->putCollar()->setFromJson(jobj["collar"].toObject());
            mCollars.append(animal);
        };

        mAnimals.append(animal);
    }

    return true;
}

float Herd::beforeGeneration( int areaDimeter, float animalSize )
{
    srand(time(NULL));

    clear();

    mAnimalSize = animalSize;
    mAnimalHalfSizeSquared = mAnimalSize * mAnimalSize * 0.25f;
    float areaRadius = areaDimeter * 0.5;
    mShepherd = new Shepherd(0.001f, areaRadius);

    return areaRadius;
}

// returns true if successful load from saved animals list
bool Herd::generate(int count,
                    int areaDimeter,
                    int percentageCollars,
                    int percentageBoluses,
                    int percentageMales,
                    float animalSize,
                    float grazingCapacity)
{
    float areaRadius = beforeGeneration( areaDimeter, animalSize);

    int collarsCount = count * percentageCollars / 100;
    int bollusesCount = count * percentageBoluses / 100;

    qDebug() << "Generating" << count << "herd in radius:" << areaRadius << "and" << collarsCount << "collars";

    mMalesCount = percentageMales * count;
    if( mMalesCount < 1 ) mMalesCount = 1;

    if( mMalesCount > MALE_NAMES_COUNT) {
        mMalesCount = MALE_NAMES_COUNT;
    }

    // fill animals array
    mAnimals.reserve(count);

    QList<int> maleNames = Animal::namesIndices(true);
    QList<int> femaleNames = Animal::namesIndices(false);

    for( auto i = 0; i < count; i ++) {

        int nameIndex;

        bool isMale = maleNames.empty() ? false : gSimTools->gen(10) > 5;

        if( isMale ) {
            nameIndex = maleNames.size() > 1 ? gSimTools->gen(maleNames.size()-1) : 0;
            maleNames.removeAt(nameIndex);
        }else {
            nameIndex = femaleNames.size() > 1 ? gSimTools->gen(femaleNames.size()-1) : 0;
            femaleNames.removeAt(nameIndex);
        }

        float x = Tools::rnd(-areaRadius, areaRadius);
        float y = Tools::rnd(-areaRadius, areaRadius);
        Animal* animal = new Animal(this, isMale, nameIndex, x, y, grazingCapacity);
        mAnimals.append(animal);

        // Put boluses only ot specified percentage
        if( i < bollusesCount ) {
            animal->putBolus();
        }

        processCollision(animalSize * 2.0f);
    }

    // fill animals with collar array
    mCollars.reserve(collarsCount);
    for( int i = 0; i < collarsCount; i ++) {
        int indexCollar = Tools::rnd(0, count);
        Animal* animal = mAnimals[indexCollar];
        while(animal->hasCollar()) {
            indexCollar ++;
            if( indexCollar >= count) {
                indexCollar = 0;
            }
            animal = mAnimals[indexCollar];
        }
        animal->putCollar();
        mCollars.append(animal);
    }

    mIsSimulation = true;

    return true;
}


bool Herd::processCollision(float collidingDistance)
{
    // process collision
    bool isCollision = false;
    foreach(Animal* animal, mAnimals) {
         foreach(Animal* otherAnimal, mAnimals) {
            // avoid collision with itself
            if( otherAnimal == animal ) {
                continue;
            }

            if( otherAnimal->collide(animal, collidingDistance ) ) {
                isCollision = true;
            }
        }
    }

    return isCollision;
}

Animal *Herd::animal(const QString &name)
{
    for( auto i = 0; i < mAnimals.count(); i ++) {
        Animal* a = mAnimals[i];
        if( a->name() == name) {
            return a;
        }
    }

    return nullptr;
}


QPointF Herd::shepherdPos()
{
    if( !mShepherd) {
        return QPointF(0, 0);
    }

    return mShepherd->lastPos();
}




bool Herd::checkTransmitVisibility(AnimalPair& ap, float maxDistanceSq, float minTransmitAngleCos)
{
    QVector2D look = ap.collarAnimal()->p() - ap.bolusAnimal()->p();
    look.normalize();

    // check weather the animal's bolus is directly visible by the collar
    // without intersection with other animals
    foreach(Animal* intersect, mAnimals) {
        // skip both animals from the pair
        if( intersect == ap.collarAnimal() || intersect == ap.bolusAnimal()) {
            continue;
        }

        // check if it's too far
        if( maxDistanceSq < ap.collarAnimal()->distanceSq(intersect) ) {
            continue;
        }

        // check angle of view
        if( !ap.collarAnimal()->isSideVisible(intersect, minTransmitAngleCos ) ) {
            continue;
        }

        // test for the intersection with the line of vision
        QVector2D lookIntersect = ap.collarAnimal()->p() - intersect->p();
        lookIntersect.normalize();

        float dot = QVector2D::dotProduct(look, lookIntersect);
        float cross = look.x()*lookIntersect.x() - look.y()*lookIntersect.x();
        float angle = std::atan2(cross, dot);
        if (angle < 0)
            angle += static_cast<float>(M_PI * 2); // make it in range 0..pi

        if( (angle < M_PI) && (ap.distanceSqToLine(intersect->pt()) < mAnimalHalfSizeSquared) ) {
            // animal pair is obscured by this animal
            return false;
        }
    }

    return true;
}


Herd::AnimalPair::AnimalPair(Animal *collarAnimal, Animal *bolusAnimal) :
    mCollarAnimal(collarAnimal), mBolusAnimal(bolusAnimal)
{
    mLine = QLineF(collarAnimal->pt(), bolusAnimal->pt());
}

float Herd::AnimalPair::distanceSqToLine(const QPointF &pt)
{
    // Vector math
    QPointF a = mLine.p1();
    QPointF b = mLine.p2();
    QPointF ap = pt - a;
    QPointF ab = b - a;

    float ab2 = QPointF::dotProduct(ab, ab);
    float ap_ab = QPointF::dotProduct(ap, ab);
    float t = ap_ab / ab2;

    // Clamp t to [0,1] if you want distance to *segment* instead of infinite line
    t = qBound(0.0, t, 1.0);

    QPointF proj = a + t * ab;
    float dx = pt.x() - proj.x();
    float dy = pt.y() - proj.y();
    return dx * dx + dy * dy;
}

void Herd::AnimalPair::appendTo(QList<AnimalPair> &ls)
{
    mBolusAnimal->addObserver(mCollarAnimal);
    mCollarAnimal->addObserving(mBolusAnimal);
    ls.append(*this);
}


QString Herd::jsonAnimalsList( bool isDevicesList )
{
    QString result = "[";
    bool hasInfoPrev = false;
    foreach(Animal* animal, mAnimals) {

        QString info = animal->jsonInfo(isDevicesList);

        bool hasInfo = !info.isEmpty();

        if( hasInfoPrev && hasInfo) {
            result.append(",");
        }

        if( hasInfo ) {
            result.append( info );
            hasInfoPrev = true;
        }
    }

    result.append("]");

    return result;
}

QStringList Herd::names()
{
    QStringList names(animalsCount());
    for( auto i = 0; i < animalsCount(); i++) {
        names.append(animal(i)->name());
    }

    return names;
}

bool Herd::storeAnimals(const QString& dir)
{
    qDebug() << "Storing herd lists in:" << dir;

    // If cannot load params from animals list - save animals list
    QString filePath = dir + (mIsSimulation ? ANIMALS_LIST_FILE_SIM : ANIMALS_LIST_FILE);
    if( !gSimTools->fileWrite(filePath, jsonAnimalsList(false).toUtf8(), true)) {
        qCritical() << "Error saving file" << filePath;
        return false;
    }

    qInfo() << "Animals lists stored in" << dir;

    return true;
}

bool Herd::storeDevices(const QString& dir)
{
    qDebug() << "Storing herd lists in:" << dir;

    QString filePath = dir + (mIsSimulation ? DEVICES_LIST_FILE_SIM : DEVICES_LIST_FILE);
    if( !gSimTools->fileWrite(dir + DEVICES_LIST_FILE, jsonAnimalsList(true).toUtf8(), true) ) {
        qCritical() << "Error saving file" << filePath;
        return false;
    }

    qInfo() << "Devices lists stored in" << dir;

    return true;
}

// TODO:
Animal* Herd::newAnimal(const QString& name, bool isMale, const QString& collarEUI, const QString& bolusEUI)
{
    int x = 0;
    int y = 0;
    float grazingCapacity = ANIMAL_INITIAL_GRAZING_CAPACITY;
    Animal* animal = new Animal(this, isMale, name, x, y, grazingCapacity);
    mAnimals.append(animal);

    if( !collarEUI.isEmpty() ) {
        animal->putCollar(collarEUI.toUtf8().toHex());
    }

    if( !bolusEUI.isEmpty() ) {
        animal->putBolus(bolusEUI.toUtf8().toHex());
    }

    return animal;
}