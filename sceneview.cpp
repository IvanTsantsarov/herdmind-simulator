#include <QMouseEvent>
#include <QWheelEvent>

#include "sceneview.h"


SceneView::SceneView(QGraphicsScene *scene, QWidget *parent) :
    QGraphicsView(scene, parent)
{
    setVisible(true);

    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::NoDrag);    // we'll handle dragging manually
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);

    scale(1, -1);

    scale(10, 10);

}

void SceneView::mousePressEvent(QMouseEvent *event)
{
    QPoint pt = event->pos();

    switch(event->button()) {
    case Qt::LeftButton:
        setCursor(Qt::ClosedHandCursor);
        mIsLeftPress = true;
        mLeftPosStart = mLeftPos = mapToScene(pt);
        break;
    case Qt::RightButton:
        setCursor(Qt::SizeAllCursor);
        mIsRightPress = true;
        mRightPosStart = mRightPos = mapToScene(pt);
        break;
    case Qt::MiddleButton:
        mIsMiddlePress = true;
        mMiddlePosStart = mMiddlePos = mapToScene(pt);
        break;
    }

     QGraphicsView::mousePressEvent(event);
}

void SceneView::mouseReleaseEvent(QMouseEvent *event)
{
    switch(event->button()) {
    case Qt::LeftButton: setCursor(Qt::ArrowCursor); mIsLeftPress = false; break;
    case Qt::RightButton: setCursor(Qt::ArrowCursor); mIsRightPress = false; break;
    case Qt::MiddleButton: setCursor(Qt::ArrowCursor); mIsMiddlePress = false; break;
    }

     QGraphicsView::mouseReleaseEvent(event);
}

void SceneView::mouseMoveEvent(QMouseEvent *event)
{
    QPoint pt = event->pos();

    if( mIsLeftPress )  {

        mLeftPos = mapToScene(pt);
        float dx = mLeftPos.x() - mLeftPosStart.x();
        float dy = mLeftPos.y() - mLeftPosStart.y();

        mLeftPosStart = mLeftPos;

        translate(dx, dy);
        event->accept();
    }
    if( mIsRightPress ) {
        mRightPos = mapToScene(pt);
    }

    if( mIsMiddlePress ) {
        mMiddlePos = mapToScene(pt);
    }

     QGraphicsView::mouseMoveEvent(event);
}

void SceneView::wheelEvent(QWheelEvent *event)
{
    // setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    float angle = event->angleDelta().y();

    float factor = qPow(1.0004, angle);

    scale(factor, factor);

     // QGraphicsView::wheelEvent(event);
}
