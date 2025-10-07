#include <QGraphicsPolygonItem>
#include "scene.h"
#include "herd.h"
#include "animal.h"
#include "defines.h"

#define ITEM_WIDTH_HALF (ANIMAL_WIDTH * 0.5f)
#define ITEM_LENGTH_HALF (ANIMAL_LENGTH * 0.5f)
#define ITEM_PEN_WIDTH (ANIMAL_LENGTH * 0.1f)

#define ITEM_PEN QPen(QColor(100, 100, 100, 100), ITEM_PEN_WIDTH)
#define ITEM_BRUSH QBrush(QColor(200, 100, 100, 100))

#define ITEM_PEN_SEL QPen(QColor(100, 100, 100, 100), ITEM_PEN_WIDTH )
#define ITEM_BRUSH_SEL QBrush(QColor(200, 100, 100, 100))

void Scene::clear()
{
    foreach(QGraphicsItem * item, items()) {
        removeItem(item);
        delete item;
    }

    mItemSelected = nullptr;
}

Scene::Scene(QObject *parent)
    : QGraphicsScene{parent}
{}

void Scene::create(int itemsCount)
{
    clear();

    // Make a triangle
    QList<QPointF> points;
    points.reserve(3);
    points.append(QPointF(-ITEM_WIDTH_HALF, +ITEM_LENGTH_HALF));
    points.append(QPointF(-ITEM_WIDTH_HALF, -ITEM_LENGTH_HALF));
    points.append(QPointF(ANIMAL_LENGTH, 0));
    QPolygonF triangle(points);

    mItems.reserve(itemsCount);
    for( auto i = 0; i < itemsCount; i ++) {
        QGraphicsPolygonItem* item = addPolygon(triangle);
        mItems.append(item);
        item->setPen(ITEM_PEN);
        item->setBrush(ITEM_BRUSH);
    }
}

void Scene::update(Herd *herd, bool isSetColor, float diameter )
{
    for( auto i = 0; i < mItems.count(); i++) {
        QGraphicsPolygonItem* item = mItems[i];
        Animal* animal = herd->animal(i);
        item->setPos(animal->point());
        item->setRotation( qRadiansToDegrees(animal->rotationAngle()));

        if( isSetColor ) {
            float w = sceneRect().width();
            float h = sceneRect().width();

            int cx = (animal->point().x() + diameter * 0.5f) / diameter * 255;
            int cy = (animal->point().y() + diameter * 0.5f) / diameter * 255;

            QColor col(cx, 100, cy, 100);
            item->setBrush(col);
            item->setPen( QPen(col, ITEM_PEN_WIDTH) );
        }
    }
}

void Scene::onFigurePick(QGraphicsPolygonItem *item, QPointF pos)
{
    if( mItemSelected ) {
        mItemSelected->setPen(ITEM_PEN);
        mItemSelected->setBrush(ITEM_BRUSH);
    }
    mItemSelected = item;
    mItemSelected->setPen(ITEM_PEN_SEL);
    mItemSelected->setBrush(ITEM_BRUSH_SEL);
}

void Scene::onFigureMove(QGraphicsPolygonItem *item, QPointF pos)
{

}

void Scene::onFigureDrop(QGraphicsPolygonItem *item, QPointF pos)
{

}
