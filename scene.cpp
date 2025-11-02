#include <QStyleOptionGraphicsItem>
#include <QVariantAnimation>

#include "scene.h"
#include "herd.h"
#include "animal.h"
#include "sceneview.h"
#include "defines.h"
#include "hardware/bolus/bolus.h"

#define ITEM_WIDTH_HALF (ANIMAL_WIDTH * 0.5f)
#define ITEM_LENGTH_HALF (ANIMAL_LENGTH * 0.5f)
#define ITEM_PEN_WIDTH (ANIMAL_LENGTH * 0.1f)
#define ITEM_PEN_WIDTH_COLLAR (ITEM_PEN_WIDTH * 2.5f)

#define ANIMAL_OPACITY 220
#define ITEM_PEN QPen(QColor(200, 200, 200, ANIMAL_OPACITY),  ITEM_PEN_WIDTH)
#define ITEM_BRUSH QBrush(QColor(250, 150, 150, ANIMAL_OPACITY))

#define PAIR_PEN QPen(QColor(250, 200, 100, 200),  ITEM_PEN_WIDTH, Qt::DashLine)
#define PAIR_PEN_SENDING QPen(QColor(250, 250, 200, 255),  ITEM_PEN_WIDTH * 2, Qt::SolidLine)

#define ITEM_PEN_SEL QPen(QColor(50, 50, 255, ANIMAL_OPACITY), ITEM_PEN_WIDTH )
#define ITEM_BRUSH_SEL QBrush(QColor(200, 100, 100, ANIMAL_OPACITY))

#define ATTRACTOR_PEN QPen(QColor(200, 0, 0, ANIMAL_OPACITY),  ITEM_PEN_WIDTH)
#define ATTRACTOR_BRUSH QBrush(QColor(200, 0, 0, ANIMAL_OPACITY))

#define LAWN_BRUSH_OPACITY 100
//#define LAWN_BRUSH_COLOR_FULL QColor(0, 25, 10)
//#define LAWN_BRUSH_COLOR_DEPLETED QColor(20, 10, 0)

#define LAWN_BRUSH_COLOR_FULL QColor(96, 153, 98)
#define LAWN_BRUSH_COLOR_DEPLETED QColor(99, 96, 88)

#define INFO_TEXT_COLOR QColor(250, 220, 100)
#define INFO_BACK_COLOR QColor(30, 30, 30, 150)

void Scene::clear()
{
    foreach(QGraphicsItem * item, items()) {
        removeItem(item);
        delete item;
    }

    mItems.clear();
    mLines.clear();
    mLawns.clear();

    mItemSelected = nullptr;
}

Scene::Scene(QObject *parent)
    : QGraphicsScene{parent}
{}

void Scene::create(SceneView* view, Herd* herd, int animalsCount, int pairsCount, int lawnsCount )
{
    // An important line - artefacts are gone with it:
    view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    clear();

    // Create the meadow
    mLawns.reserve(lawnsCount);
    for( auto i = 0; i < lawnsCount; i ++) {
        QGraphicsRectItem* r = addRect(-LAWN_RADIUS, -LAWN_RADIUS,
                                       LAWN_DIAMETER, LAWN_DIAMETER,
                                       Qt::NoPen, LAWN_BRUSH_COLOR_FULL);

        r->setFlag(QGraphicsItem::ItemIsSelectable, false);
        r->setFlag(QGraphicsItem::ItemIsFocusable, false);
        r->setFlag(QGraphicsItem::ItemIsMovable, false);
        r->setFlag(QGraphicsItem::ItemIsFocusScope, false);
        r->setFlag(QGraphicsItem::ItemIsPanel, false);
        mLawns.append(r);

    }

    // Make a triangle
    QList<QPointF> points;
    points.reserve(3);
    points.append(QPointF(-ITEM_WIDTH_HALF, +ITEM_LENGTH_HALF));
    points.append(QPointF(-ITEM_WIDTH_HALF, -ITEM_LENGTH_HALF));
    points.append(QPointF(ANIMAL_LENGTH, 0));
    QPolygonF triangle(points);

    // Create animals figures
    mItems.reserve(animalsCount);
    for( auto i = 0; i < animalsCount; i ++) {
        AnimalItem* item = new AnimalItem(herd->animal(i), triangle);
        addItem(item);
        mItems.append(item);

        item->setCacheMode(QGraphicsItem::NoCache);
        item->setPen(ITEM_PEN);
        item->setBrush(ITEM_BRUSH);
        item->setFlag(QGraphicsItem::ItemIsSelectable, true);
        item->setFlag(QGraphicsItem::ItemIsFocusable, false);
        item->setAcceptHoverEvents(true);
    }

    // Create animals conenctions lines
    mLines.reserve(pairsCount);
    for( auto i = 0; i < pairsCount; i ++) {
        QGraphicsLineItem* line = addLine(0, 0, 1, 1, PAIR_PEN);
        mLines.append(line);
        line->hide();
    }

    mAttractor = addEllipse(0, 0, ANIMAL_LENGTH, ANIMAL_LENGTH, ATTRACTOR_PEN, ATTRACTOR_BRUSH );

    mItemInfo = new TextItem("");
    mItemInfo->setDefaultTextColor(INFO_TEXT_COLOR);
    mItemInfo->setBackColor(INFO_BACK_COLOR);
    QTransform t = mItemInfo->transform();
    t.scale(0.1f, -0.1f);
    mItemInfo->setTransform(t);
    addItem(mItemInfo);

    QFont font("Monospace");
    font.setStyleHint(QFont::Monospace);
    font.setPointSize(mItemInfo->font().pointSize());
    mItemInfo->setFont(font);

    setBackgroundBrush(LAWN_BRUSH_COLOR_DEPLETED);
}

void Scene::update(Herd *herd, Meadow *meadow, bool isInitial, float diameter )
{
    // update lawns
    int lawnIndex = 0;

    //mSceneInfo->setPlainText(QString("%1%").arg(meadow->kgRatio() * 100.0f, 0, 'f', 1));


    if( mItemSelected ) {
        mItemInfo->show();
        mItemInfo->setPlainText(mItemSelected->animal()->info());
        mItemInfo->setPos(mItemSelected->pos());
    }else {
        mItemInfo->hide();
    }

    foreach(Meadow::Lawn* lawn, meadow->lawns()) {
        QGraphicsRectItem* r = mLawns[lawnIndex++];


        if( isInitial || lawn->animalsCount() ) {

            if( isInitial ) {
                r->setPos(lawn->pos());
            }

            QColor color = LAWN_BRUSH_COLOR_FULL;
            color.setAlpha( lawn->kgNorm() * 255.0f );
            r->setBrush(color);
        }
    }


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
        item->setToolTip(animal->info());

        if( isInitial ) {

            int cx = (animal->pt().x() + diameter * 0.5f) / diameter * 200;
            int cy = (animal->pt().y() + diameter * 0.5f) / diameter * 200;

            QColor colBrush(cx, 30, cy, 150);
            QColor colPen(cx, 30, cy, 250);
            item->setBrush(colBrush);
            item->setPen( animal->hasCollar() ?
                             QPen(Qt::white, ITEM_PEN_WIDTH_COLLAR) :
                             QPen(colPen, ITEM_PEN_WIDTH) );
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
        line->setPen( pair.bolusAnimal()->bolus()->isSendingData() ? PAIR_PEN_SENDING : PAIR_PEN );
        line->show();
    }

}

void Scene::selectAnimalItem(AnimalItem *item)
{
    clearSelection();
    clearFocus();

    if( mItemSelected ) {
        mItemSelected->setSelected(false);

        // deselect current item
        if( item == mItemSelected ) {
            mItemSelected = nullptr;
            return;
        }
    }

    mItemSelected = item;

    mItemSelected->ensureVisible();
}

void Scene::selectAnimalItem(int index)
{
    if( index >= mItems.length() ) {
        return;
    }

    AnimalItem* item = mItems[index];
    selectAnimalItem(item);
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

    if (animalScene()->selectedAnimal() == this) {

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, false);

        // Draw a custom selection border
        QPen pen(ITEM_PEN_SEL);
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



QVariant AnimalItem::itemChange(GraphicsItemChange change, const QVariant &v) {
    if (change == ItemSelectedHasChanged) {
        // if (v.toBool()) startPulseAnimation();   // trigger when selected
        if( v.toBool() ) {
            animalScene()->selectAnimalItem(this);
        }
    }
    return QGraphicsPolygonItem::itemChange(change, v);
}

void TextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *w) {
    painter->setBrush(mBackColor);
    painter->setPen(ITEM_PEN_SEL);
    painter->drawRect(boundingRect());
    QGraphicsTextItem::paint(painter, o, w);
}
