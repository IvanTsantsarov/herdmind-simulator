#ifndef HERD_H
#define HERD_H

#include <QObject>
#include <QVector2D>

class Animal;

class Herd : public QObject
{
    Q_OBJECT
    QVector<Animal*> mAnimals;
    void clear();
public:
    explicit Herd(QObject *parent = nullptr);
    void generate(int count, int areaDimeter);
    void update(QPointF *attractor,
                float attractorPower,
                float attractionDistance, float repellingDistance,
                float collidingDistance,
                float maxSpeed , float friction);

    inline Animal* animal(int index){ return mAnimals[index]; }
signals:
};

#endif // HERD_H
