#ifndef SHEPHERD_H
#define SHEPHERD_H

#include <QObject>
#include <QPointF>

#define SHEPHERD_COEF_COUNT 10

class Shepherd : public QObject
{
    Q_OBJECT
    float mStep  = 0.0f;
    float mRadius = 0.0f;
    float mPos  = 0.0f;
    float mK[10];
    QPointF mLastPos;

public:
    explicit Shepherd(float step, float radius, QObject *parent = nullptr);
    void reset();
    QPointF step();
    inline QPointF lastPos(){  return mLastPos; }


signals:
};



#endif // SHEPHERD_H
