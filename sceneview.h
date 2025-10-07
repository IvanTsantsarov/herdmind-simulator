#ifndef SCENEVIEW_H
#define SCENEVIEW_H

#include <QGraphicsView>

class SceneView : public QGraphicsView
{
    QPointF mLeftPos, mRightPos, mMiddlePos;
    QPointF mLeftPosStart, mRightPosStart, mMiddlePosStart;
    bool mIsLeftPress = false;
    bool mIsRightPress = false;
    bool mIsMiddlePress = false;

public:
    SceneView(QGraphicsScene *scene, QWidget *parent = nullptr);

    bool isLeftPress(){ return mIsLeftPress; };
    bool isRightPress(){ return mIsRightPress; };
    bool isMiddlePress(){ return mIsMiddlePress; };

    QPointF leftPos(){ return mLeftPos;}
    QPointF rightPos(){ return mRightPos;}
    QPointF middlePos(){ return mMiddlePos;}

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent *event);

};

#endif // SCENEVIEW_H
