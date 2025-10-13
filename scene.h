#ifndef SCENE_H
#define SCENE_H


#include <QObject>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsPolygonItem>

class QStyleOptionGraphicsItem;
class QGraphicsItem;

class Herd;

class AnimalItem : public QObject, public QGraphicsPolygonItem {
    Q_OBJECT

    float mScale = 0.0;
public:
    AnimalItem( const QPolygonF& poly) : QGraphicsPolygonItem(poly) {}
    AnimalItem(const QRectF &rect, QGraphicsItem *parent = nullptr)
        : QGraphicsPolygonItem(rect, parent) {}

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void startPulseAnimation();
protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &v) override {
        if (change == ItemSelectedHasChanged) {
            if (v.toBool()) startPulseAnimation();   // trigger when selected
        }
        return QGraphicsPolygonItem::itemChange(change, v);
    }
};


class Scene : public QGraphicsScene
{
    Q_OBJECT

    QVector<AnimalItem*> mItems;
    QVector<QGraphicsLineItem*> mLines;
    AnimalItem* mItemSelected = nullptr;

    QGraphicsEllipseItem* mAttractor = nullptr;

    void clear();
public:
    explicit Scene(QObject *parent = nullptr);

    void create(int itemsCount, int pairsCount);
    void update(Herd* herd, bool isSetColor = false, float diameter = 0.0f );

    void selectFigure(int index);


public slots:
    void onFigurePick(QGraphicsPolygonItem* item, QPointF pos);
    void onFigureMove(QGraphicsPolygonItem *item, QPointF pos);
    void onFigureDrop(QGraphicsPolygonItem *item, QPointF pos);

};

#endif // SCENE_H
