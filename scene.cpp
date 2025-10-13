#include <QStyleOptionGraphicsItem>
#include <QVariantAnimation>

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

#define PAIR_PEN QPen(QColor(100, 100, 100, 100),  ITEM_PEN_WIDTH)

#define ITEM_PEN_SEL QPen(QColor(100, 100, 100, 100), ITEM_PEN_WIDTH )
#define ITEM_BRUSH_SEL QBrush(QColor(200, 100, 100, 100))


#define ATTRACTOR_PEN QPen(QColor(200, 0, 0, 200),  ITEM_PEN_WIDTH)
#define ATTRACTOR_BRUSH QBrush(QColor(200, 0, 0, 100))


void Scene::clear()
{
    foreach(QGraphicsItem * item, items()) {
        removeItem(item);
        delete item;
    }

    mItems.clear();
    mLines.clear();

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

    // Create animals figures
    mItems.reserve(itemsCount);
    for( auto i = 0; i < itemsCount; i ++) {
        AnimalItem* item = new AnimalItem(triangle);
        addItem(item);
        mItems.append(item);

        item->setCacheMode(QGraphicsItem::NoCache);
        item->setPen(ITEM_PEN);
        item->setBrush(ITEM_BRUSH);
        item->setFlag(QGraphicsItem::ItemIsSelectable, true);
        item->setFlag(QGraphicsItem::ItemIsFocusable, false);
    }

    // Create animals conenctions lines
    mLines.reserve(pairsCount);
    for( auto i = 0; i < pairsCount; i ++) {
        QGraphicsLineItem* line = addLine(0, 0, 1, 1, PAIR_PEN);
        mLines.append(line);
        line->hide();
    }

    mAttractor = addEllipse(0, 0, ANIMAL_LENGTH, ANIMAL_LENGTH, ATTRACTOR_PEN, ATTRACTOR_BRUSH );
}

void Scene::update(Herd *herd, bool isSetColor, float diameter )
{
    if( herd->isShepherdActive() ) {
        mAttractor->show();
        mAttractor->setPos(herd->shepherdPos());
    }else {
        mAttractor->hide();
    }

    for( auto i = 0; i < mItems.count(); i++) {
        QGraphicsPolygonItem* item = mItems[i];
        Animal* animal = herd->animal(i);
        item->setPos(animal->pt());

        item->setRotation( qRadiansToDegrees(animal->rotationAngle()));

        if( isSetColor ) {

            int cx = (animal->pt().x() + diameter * 0.5f) / diameter * 255;
            int cy = (animal->pt().y() + diameter * 0.5f) / diameter * 255;

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
        line->setLine(QLineF(pair.bolusAnimal()->pt(), pair.collarAnimal()->pt()));
        line->show();
    }

}

void Scene::selectFigure(int index)
{
    clearSelection();
    clearFocus();

    if( mItemSelected ) {
        mItemSelected->setSelected(false);
    }

    if( index >= mItems.length() ) {
        return;
    }

    AnimalItem* item = mItems[index];

    mItemSelected = item;
    mItemSelected->setSelected(true);

    mItemSelected->ensureVisible();
}

void Scene::onFigurePick(QGraphicsPolygonItem *item, QPointF pos)
{
    (void)item;
    (void)pos;
}

void Scene::onFigureMove(QGraphicsPolygonItem *item, QPointF pos)
{
    (void)item;
    (void)pos;
}

void Scene::onFigureDrop(QGraphicsPolygonItem *item, QPointF pos)
{
    (void)item;
    (void)pos;
}

void AnimalItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {

    QStyleOptionGraphicsItem opt(*option);
    opt.state &= ~QStyle::State_Selected;   // suppress default selection outline
    opt.state &= ~QStyle::State_HasFocus;   // suppress focus cue
    QGraphicsPolygonItem::paint(painter, &opt, widget);

    if (isSelected()) {
        painter->save();

        // Draw a custom selection border
        QPen pen(Qt::blue);
        pen.setWidthF(2.0);
        pen.setCosmetic(true);            // keep width independent of zoom
        pen.setStyle(Qt::SolidLine);    // any style you like
        painter->setPen(pen);

        const QRectF br = polygon().boundingRect();
        const QPointF c = br.center();

        painter->translate(c);
        painter->scale(1.0 + mScale, 1.0 + mScale);
        painter->translate(-c);

        // painter->setPen(QPen(Qt::blue, ITEM_PEN_WIDTH));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect());

        painter->restore();
    }
}

void AnimalItem::startPulseAnimation() {
    auto *anim = new QVariantAnimation(this);
    anim->setDuration(1000); // ms
    anim->setStartValue(1.0);
    anim->setKeyValueAt(0.5, 0.0); // fade out mid-way
    anim->setEndValue(1.0);        // fade back in
    anim->setEasingCurve(QEasingCurve::InOutSine);

    connect(anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        mScale = v.toReal();
        update();
    });
    connect(anim, &QVariantAnimation::finished, anim, &QObject::deleteLater);
    anim->start();
}
