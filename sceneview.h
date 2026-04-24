#ifndef SCENEVIEW_H
#define SCENEVIEW_H

#include <QGraphicsView>

class Scene;
class Meadow;

class SceneView : public QGraphicsView
{
public:
    enum Mode {
        Explore = 0,
        Fence
    };

    void setMode( Mode mode );
private:
    Mode mMode = Mode::Explore;

    QPoint mMousePoint;
    QPointF mMousePointScene;

    QPointF mLeftPos, mRightPos, mMiddlePos;
    QPointF mLeftPosStart, mRightPosStart, mMiddlePosStart;
    bool mIsLeftPress = false;
    bool mIsRightPress = false;
    bool mIsMiddlePress = false;

    Scene* mScene = nullptr;
    Meadow* mMeadow = nullptr;

public:
    SceneView(QGraphicsScene *scene, QWidget *parent = nullptr);

    void setMeadow(Meadow* meadow){ mMeadow = meadow; }

    bool isLeftPress(){ return mIsLeftPress; };
    bool isRightPress(){ return mIsRightPress; };
    bool isMiddlePress(){ return mIsMiddlePress; };

    QPointF leftPos(){ return mLeftPos;}
    QPointF rightPos(){ return mRightPos;}
    QPointF middlePos(){ return mMiddlePos;}

    void updateCursorInfo();

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent *event);
    void resizeEvent(QResizeEvent* event);

    void drawForeground(QPainter* painter, const QRectF& rect) override;

};

#endif // SCENEVIEW_H
