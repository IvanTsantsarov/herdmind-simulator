#include <QGraphicsPolygonItem>
#include "scene.h"
#include "herd.h"
#include "animal.h"
#include "defines.h"

#define ITEM_WIDTH_HALF (ANIMAL_WIDTH * 0.5f)
#define ITEM_LENGTH_HALF (ANIMAL_LENGTH * 0.5f)
#define ITEM_PEN_WIDTH (ANIMAL_LENGTH * 0.1f)
#define ITEM_PEN_WIDTH_COLLAR (ANIMAL_LENGTH * 0.5f)

#define ITEM_PEN QPen(QColor(100, 100, 100, 100),  ITEM_PEN_WIDTH)
#define ITEM_BRUSH QBrush(QColor(200, 100, 100, 100))

#define PAIR_PEN QPen(QColor(100, 100, 100, 100),  ANIMAL_WIDTH)

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

void Scene::create(int itemsCount, int pairsCount )
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

    mLines.reserve(pairsCount);
    for( auto i = 0; i < pairsCount; i ++) {
        QGraphicsLineItem* line = addLine(0, 0, 1, 1, PAIR_PEN);
        mLines.append(line);
        line->hide();
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

            int cx = (animal->point().x() + diameter * 0.5f) / diameter * 255;
            int cy = (animal->point().y() + diameter * 0.5f) / diameter * 255;

            QColor col(cx, 100, cy, 100);
            item->setBrush(col);
            item->setPen( animal->hasCollar() ?
                             QPen(Qt::black, ITEM_PEN_WIDTH_COLLAR) :
                             QPen(col, ITEM_PEN_WIDTH) );
        }
    }

    // hide all lines
    for( auto i = 0; i < mLines.count(); i ++) {
        mLines[i]->hide();
    }

    // show pair lines
    Herd::PairsList pairs = herd->infoPairs();
    int activatedLine = 0;
    foreach(Herd::AnimalPair pair, pairs) {
        QGraphicsLineItem* line = mLines[activatedLine++];
        line->setLine(QLineF(pair.first()->pt(), pair.second()->pt()));
        line->show();
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
