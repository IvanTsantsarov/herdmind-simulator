#include <QStyleOptionGraphicsItem>
#include <QVariantAnimation>

#include "scene.h"
#include "herd.h"
#include "animal.h"
#include "sceneview.h"
#include "defines.h"
#include "hardware/bolus/bolus.h"
#include "hardware/gateway/gateway.h"
#include "network.h"

#define ITEM_WIDTH_HALF (ANIMAL_WIDTH * 0.5f)
#define ITEM_LENGTH_HALF (ANIMAL_LENGTH * 0.5f)
#define ITEM_PEN_WIDTH (ANIMAL_LENGTH * 0.1f)
#define ITEM_PEN_WIDTH_COLLAR (ITEM_PEN_WIDTH * 2.5f)

#define ANIMAL_OPACITY 220
#define ITEM_PEN QPen(QColor(200, 200, 200, ANIMAL_OPACITY),  ITEM_PEN_WIDTH)
#define ITEM_BRUSH QBrush(QColor(250, 150, 150, ANIMAL_OPACITY))

#define BOLUS_PAIR_PEN QPen(QColor(250, 200, 100, 200),  ITEM_PEN_WIDTH, Qt::DashLine)
#define BOLUS_PAIR_PEN_SENDING QPen(QColor(250, 250, 200, 255),  ITEM_PEN_WIDTH * 2, Qt::SolidLine)

#define GATEWAY_PAIR_PEN QPen(QColor(250, 150, 150, 200),  ITEM_PEN_WIDTH, Qt::DotLine)
#define GATEWAY_PAIR_PEN_SENDING QPen(QColor(250, 250, 200, 255),  ITEM_PEN_WIDTH * 2, Qt::SolidLine)

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

    mAnimalItems.clear();
    mLinesAnimals.clear();

    mGatewayItems.clear();
    mLinesGateways.clear();

    mLawns.clear();

    mAnimalItemSelected = nullptr;
}

Scene::Scene(QObject *parent)
    : QGraphicsScene{parent}
{}

void Scene::create(SceneView* view, Herd* herd, Network* network,
                   int lawnsCount, int collarPairsCount, int gatewayPairsCount )
{
    mView = view;

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

    // Create a triangle for the animal poly
    QList<QPointF> points;
    points.reserve(3);
    points.append(QPointF(-ITEM_WIDTH_HALF, +ITEM_LENGTH_HALF));
    points.append(QPointF(-ITEM_WIDTH_HALF, -ITEM_LENGTH_HALF));
    points.append(QPointF(ANIMAL_LENGTH, 0));
    QPolygonF triangle(points);

    int animalsCount = herd->animalsCount();

    // Create animals figures
    mAnimalItems.reserve(animalsCount);
    for( auto i = 0; i < animalsCount; i ++) {
        AnimalItem* item = new AnimalItem(herd->animal(i), triangle);
        addItem(item);
        mAnimalItems.append(item);
    }

    // Create bolus-collars conenctions lines
    mLinesAnimals.reserve(collarPairsCount);
    for( auto i = 0; i < collarPairsCount; i ++) {
        QGraphicsLineItem* line = addLine(0, 0, 1, 1, BOLUS_PAIR_PEN);
        mLinesAnimals.append(line);
        line->hide();
    }


    // Create a hexagon for the gateway poly
    points.clear();
    points.reserve(6);
    for( auto i = 0; i < 6; i++) {
        float a = i * M_PI / 3.0f;
        points.append(QPointF(sinf(a)*ANIMAL_WIDTH, cosf(a)*ANIMAL_WIDTH ));
    }
    QPolygonF poly(points);

    // Create animals figures
    mGatewayItems.reserve(network->gatewaysCount());
    for( auto i = 0; i < network->gatewaysCount(); i ++) {
        GatewayItem* item = new GatewayItem(network->gateway(i), poly);
        item->setPen(ITEM_PEN);
        item->setBrush(ITEM_BRUSH);
        addItem(item);
        mGatewayItems.append(item);
    }


    // Create collars-gateways conenctions lines
    mLinesGateways.reserve(gatewayPairsCount);
    for( auto i = 0; i < gatewayPairsCount; i ++) {
        QGraphicsLineItem* line = addLine(0, 0, 1, 1, BOLUS_PAIR_PEN);
        mLinesGateways.append(line);
        line->hide();
    }

    mAttractor = addEllipse(0, 0, ANIMAL_LENGTH, ANIMAL_LENGTH, ATTRACTOR_PEN, ATTRACTOR_BRUSH );

    mItemInfo = new TextItem("", this);
    mCursorInfo = new TextItem("", this);

    setBackgroundBrush(LAWN_BRUSH_COLOR_DEPLETED);
}

void Scene::update(Herd *herd, Meadow *meadow, Network* network, bool isInitial, float diameter )
{
    if( !mAttractor) {
        return;
    }

    // update lawns
    int lawnIndex = 0;

    //mSceneInfo->setPlainText(QString("%1%").arg(meadow->kgRatio() * 100.0f, 0, 'f', 1));


    if( mItemInfo ) {
        if( mAnimalItemSelected ) {
            mItemInfo->show();
            mItemInfo->setPlainText(mAnimalItemSelected->animal()->info());
            mItemInfo->setPos(mAnimalItemSelected->pos());
        }else {
            mItemInfo->hide();
        }
    }

    foreach(Meadow::Lawn* lawn, meadow->lawns()) {
        QGraphicsRectItem* r = mLawns[lawnIndex++];

        if( isInitial || lawn->animalsCount() || lawn->mustUpdate ) {

            if( isInitial ) {
                r->setPos(lawn->pos());
            }

            QColor color = LAWN_BRUSH_COLOR_FULL;
            color.setAlpha( lawn->kgNorm() * 255.0f );
            r->setBrush(color);
            mView->updateCursorInfo();
            lawn->mustUpdate = false;
        }
    }


    if( herd->isShepherdActive() ) {
        mAttractor->show();
        mAttractor->setPos(herd->shepherdPos());
    }else {
        mAttractor->hide();
    }

    for( auto i = 0; i < mAnimalItems.count(); i++) {
        QGraphicsPolygonItem* item = mAnimalItems[i];
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
    for( auto i = 0; i < mLinesAnimals.count(); i ++) {
        QGraphicsLineItem* line = mLinesAnimals[i];
        line->setPen( BOLUS_PAIR_PEN );
        line->hide();
    }

    // show pair lines
    Herd::PairsListBC& pairsBC = herd->pairs();
    int activatedLine = 0;
    foreach(Herd::AnimalPair pair, pairsBC) {
        QGraphicsLineItem* line = mLinesAnimals[activatedLine++];
        line->setLine(QLineF(pair.bolusAnimal()->pt(), pair.collarAnimal()->pt()));
        line->setPen( pair.bolusAnimal()->bolus()->isSending() ? BOLUS_PAIR_PEN_SENDING : BOLUS_PAIR_PEN );
        line->show();
    }

    for( auto i = 0; i < mGatewayItems.count(); i ++) {
        mGatewayItems[i]->setPos( network->gateway(i)->mPos );
    }

    // hide all lines
    for( auto i = 0; i < mLinesGateways.count(); i ++) {
        QGraphicsLineItem* line = mLinesGateways[i];
        line->setPen( GATEWAY_PAIR_PEN );
        line->hide();
    }

    // show pair lines
    Network::PairsListG& pairsG = network->pairs();
    activatedLine = 0;
    foreach(Network::GatewayPair pair, pairsG) {
        QGraphicsLineItem* line = mLinesGateways[activatedLine++];
        line->setLine(QLineF(pair.firstPt(), pair.secondPt()));
        line->setPen( pair.isSending() ? GATEWAY_PAIR_PEN_SENDING : GATEWAY_PAIR_PEN );
        line->show();
    }


}

void Scene::selectAnimalItem(AnimalItem *item)
{
    clearSelection();
    clearFocus();

    if( mAnimalItemSelected ) {
        mAnimalItemSelected->setSelected(false);

        // deselect current item
        if( item == mAnimalItemSelected ) {
            mAnimalItemSelected = nullptr;
            return;
        }
    }

    mAnimalItemSelected = item;

    mAnimalItemSelected->ensureVisible();
}

void Scene::selectGatewayItem(GatewayItem *item)
{
    if( mAnimalItemSelected ) {
        mAnimalItemSelected->setSelected(false);

        // deselect current item
        if( item == mGatewayItemSelected ) {
            mGatewayItemSelected = nullptr;
            return;
        }
    }

    mGatewayItemSelected = item;

    mGatewayItemSelected->ensureVisible();
}

void Scene::setCursorInfoPos(const QPointF &pt, float kg)
{
    if( mCursorInfo ) {
        mCursorInfo->setPos(pt);
        QString str = QString("%1 %2 %3")
                          .arg( pt.x(), 0, 'f', 2)
                          .arg( pt.y(), 0, 'f', 2)
                          .arg( kg, 0, 'f', 2);

        mCursorInfo->setPlainText(str);
    }
}

void Scene::selectAnimalItem(int index)
{
    if( index >= mAnimalItems.length() ) {
        return;
    }

    AnimalItem* item = mAnimalItems[index];
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


SelectableItem::SelectableItem(const QPolygonF &poly) : QGraphicsPolygonItem(poly) {
    // setFlag(QGraphicsItem::ItemClipsToShape, true);
    setCacheMode(QGraphicsItem::NoCache);
    setPen(ITEM_PEN);
    setBrush(ITEM_BRUSH);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsFocusable, false);
    setAcceptHoverEvents(true);
}

void SelectableItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {

    QStyleOptionGraphicsItem opt(*option);
    opt.state &= ~QStyle::State_Selected;   // suppress default selection outline
    opt.state &= ~QStyle::State_HasFocus;   // suppress focus cue
    QGraphicsPolygonItem::paint(painter, &opt, widget);

    if (parentScene()->selectedAnimal() == this) {

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

void SelectableItem::startPulseAnimation() {
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



QVariant SelectableItem::itemChange(GraphicsItemChange change, const QVariant &v) {
    if (change == ItemSelectedHasChanged) {
        if (v.toBool()) {
            startPulseAnimation();   // trigger when selected
        }

        if( v.toBool() ) {
            onSelection();
        }
    }
    return QGraphicsPolygonItem::itemChange(change, v);
}

TextItem::TextItem(const QString &text, Scene *scene) :
    QGraphicsTextItem(text) {
    setDefaultTextColor(INFO_TEXT_COLOR);
    setBackColor(INFO_BACK_COLOR);
    QTransform t = transform();
    t.scale(0.1f, -0.1f);
    setTransform(t);
    scene->addItem(this);

    QFont font("Monospace");
    font.setStyleHint(QFont::Monospace);
    font.setPointSize(font.pointSize());
    setFont(font);
}

void TextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *w) {
    painter->setBrush(mBackColor);
    painter->setPen(ITEM_PEN_SEL);
    painter->drawRect(boundingRect());
    QGraphicsTextItem::paint(painter, o, w);
}

void AnimalItem::onSelection() {
    parentScene()->selectAnimalItem(this);
}

void GatewayItem::onSelection()
{
    parentScene()->selectGatewayItem(this);
}
