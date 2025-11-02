#include <QMouseEvent>
#include <QWheelEvent>

#include "sceneview.h"
#include "scene.h"


#define INFO_TEXT_COLOR QColor(250, 220, 100)
#define INFO_BACK_COLOR QColor(30, 30, 30, 150)

SceneView::SceneView(QGraphicsScene *scene, QWidget *parent) :
    QGraphicsView(scene, parent), mScene(reinterpret_cast<Scene*>(scene) )
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
        mIsLeftPress = true;
        mLeftPosStart = mLeftPos = mapToScene(pt);
        break;
    case Qt::RightButton:
        setCursor(Qt::SizeAllCursor);
        mIsRightPress = true;
        mRightPosStart = mRightPos = mapToScene(pt);
        break;
    case Qt::MiddleButton:
        setCursor(Qt::ClosedHandCursor);
        mIsMiddlePress = true;
        mMiddlePosStart = mMiddlePos = mapToScene(pt);
        break;
    default:
        qDebug() << "Other mouse button pressed:" << event->button();
    }

     QGraphicsView::mousePressEvent(event);
}

void SceneView::mouseReleaseEvent(QMouseEvent *event)
{
    switch(event->button()) {
    case Qt::LeftButton: setCursor(Qt::ArrowCursor); mIsLeftPress = false; break;
    case Qt::RightButton: setCursor(Qt::ArrowCursor); mIsRightPress = false; break;
    case Qt::MiddleButton: setCursor(Qt::ArrowCursor); mIsMiddlePress = false; break;
    default:
        qDebug() << "Other mouse button released:" << event->button();

    }

     QGraphicsView::mouseReleaseEvent(event);
}

void SceneView::mouseMoveEvent(QMouseEvent *event)
{
    QPoint pt = event->pos();

    if( mIsLeftPress )  {

        mLeftPos = mapToScene(pt);
    }
    if( mIsRightPress ) {
        mRightPos = mapToScene(pt);
    }

    if( mIsMiddlePress ) {
        mMiddlePos = mapToScene(pt);

        float dx = mLeftPos.x() - mLeftPosStart.x();
        float dy = mLeftPos.y() - mLeftPosStart.y();

        mMiddlePosStart = mMiddlePos;

        translate(dx, dy);
        event->accept();

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

void SceneView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
}

void SceneView::drawForeground(QPainter *painter, const QRectF &rect) {
    QGraphicsView::drawForeground(painter, rect);
/*
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::white);
    painter->setFont(QFont("Arial", 10));

    QString text = mSceneInfo->toPlainText();

    QRectF textRect = painter->boundingRect(
        QRectF(), Qt::AlignRight | Qt::AlignTop, text
        );

    // anchor to top-right corner with a small margin
    QPointF topRight = mapToScene(viewport()->width(), 0);
    QPointF anchor = mapFromScene(topRight); // convert to view coordinates (optional)
    int margin = 8;
    painter->drawText(viewport()->width() - textRect.width() - margin,
                      margin + textRect.height(),
                      text);
    painter->restore();
*/
}
