#ifndef SCENE_H
#define SCENE_H

#include <QGraphicsScene>

class QGraphicsItem;

class Herd;

class Scene : public QGraphicsScene
{
    QVector<QGraphicsPolygonItem*> mItems;
    QGraphicsPolygonItem* mItemSelected = nullptr;

    void clear();
public:
    explicit Scene(QObject *parent = nullptr);

    void create(int itemsCount);
    void update(Herd* herd);


public slots:
    void onFigurePick(QGraphicsPolygonItem* item, QPointF pos);
    void onFigureMove(QGraphicsPolygonItem *item, QPointF pos);
    void onFigureDrop(QGraphicsPolygonItem *item, QPointF pos);

};

#endif // SCENE_H
