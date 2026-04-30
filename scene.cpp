#include <QStyleOptionGraphicsItem>
#include <QVariantAnimation>

#include "scene.h"
#include "herd.h"
#include "animal.h"
#include "sceneview.h"
#include "defines.h"
#include "hardware/bolus/bolus.h"
#include "hardware/gateway/gateway.h"
#include "hardware/defines.h"
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

#define FENCE_COLOR_ACTIVE QColor(QColor(110, 52, 235))
#define FENCE_PEN_ACTIVE QPen( QBrush(FENCE_COLOR_ACTIVE), 0.3f, Qt::DashDotLine)
#define FENCE_BRUSH_ACTIVE QBrush( FENCE_COLOR_ACTIVE, Qt::DiagCrossPattern)

#define FENCE_COLOR QColor(QColor(10, 12, 55))
#define FENCE_PEN QPen( QBrush(FENCE_COLOR), 0.3f, Qt::DashDotLine)
#define FENCE_BRUSH QBrush( FENCE_COLOR, Qt::DiagCrossPattern)

#define POPUP_TEXT_COLOR QColor(250, 150, 50)
#define POPUP_BACK_COLOR QColor(10, 10, 10)
#define POPUP_BORDER_COLOR QColor(100, 10, 10)


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

    mMeadow = nullptr;

    mAnimalItemSelected = nullptr;
}

void Scene::updateMeadowBrush()
{
    QBrush br;
    br.setTextureImage(mMeadowImage);
    QTransform tr;
    tr.reset();

    QRectF r = mMeadow->boundingRect();
    tr.translate(r.x(), r.y());
    tr.scale((float)r.width()/mMeadowImage.width(),
             (float) r.height()/mMeadowImage.height() );

    br.setTransform(tr);

    mMeadow->setBrush(br);
}

void Scene::recreateFenceItem()
{
    if( mFenceItem ) {
        delete mFenceItem;
    }

    mFenceItem = addPolygon(mFence, FENCE_PEN, FENCE_BRUSH);
}

Scene::Scene(QObject *parent)
    : QGraphicsScene{parent}
{
    mPopupTimer = new QTimer(this);
    mPopupTimer->setSingleShot(true);
    connect( mPopupTimer, &QTimer::timeout, this, &Scene::onPopupHide );
}

void Scene::create(SceneView* view, Herd* herd, Network* network,
                   QSize meadowDim, int collarPairsCount, int gatewayPairsCount )
{
    mView = view;

    // An important line - artefacts are gone with it:
    view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    clear();


    // Create the meadow
    mMeadowImage = QImage(meadowDim, QImage::Format_RGBA8888);
    // mMeadowImage = QImage("image.png");

    QSize sz(LAWN_DIAMETER*meadowDim.width(), LAWN_DIAMETER*meadowDim.height());
    mMeadow = addRect(-sz.width()/2,
                      -sz.height()/2,
                      sz.width(),
                      sz.height() );
    updateMeadowBrush();

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
    mPopup = new TextItem("", this);

    setBackgroundBrush(LAWN_BRUSH_COLOR_DEPLETED);
}

void Scene::update(Herd *herd, Meadow *meadow, Network* network, bool isInitial, float diameter )
{
    if( !mAttractor) {
        return;
    }

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

    // update lawn colors
    QSize dim = meadow->dim();
    for(int h = 0; h < dim.height(); h ++) {
        for(int w = 0; w < dim.width(); w ++) {

            // QGraphicsRectItem* r = mLawns[lawnIndex++];

            Meadow::Lawn* lawn = meadow->byIndex(w, h);
            if( !isInitial ) {
                if( !lawn->animalsCount() && !lawn->mustUpdate ) {
                    continue;
                }
            }

            // QColor color = mMeadowImage.pixelColor(w, h);
            QColor color = LAWN_BRUSH_COLOR_FULL;
            color.setAlpha( lawn->kgNorm() * 255.0f );
            mMeadowImage.setPixelColor(w, h, color);
            mView->updateCursorInfo();
            lawn->mustUpdate = false;
        }
    }

    updateMeadowBrush();


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

void Scene::setCursorInfoPos(const QPointF &pt, const QGeoCoordinate& location, float kg)
{
    if( mCursorInfo ) {
        mCursorInfo->setPos(pt);
        QString str = QString("%1 %2\n%3 %4 %5")
                          .arg( pt.x(), 0, 'f', 2)
                          .arg( pt.y(), 0, 'f', 2)
                          .arg( location.latitude(), 0, 'f', 6)
                          .arg( location.longitude(), 0, 'f', 6)
                          .arg( kg, 0, 'f', 2);

        mCursorInfo->setPlainText(str);
    }
}

bool Scene::storeImage(const QString &path)
{
    QString fileName = path.isEmpty() ? "image.png" : path;
    return mMeadowImage.save(fileName);
}

bool Scene::fenceAppend(const QPointF& pt)
{
    if( mFence.count() > VIRTUAL_FENCE_MAX_POINTS ) {
        showPopup("Maximum number of fence points reached!");
        return false;
    }

    if( mFence.count() < 3 ) {
        mFence.append(pt);
        recreateFenceItem();
        return true;
    }


    auto pointDistSq = [](const QPointF& pt1, const QPointF& pt2) {
        QPointF sub = pt2 - pt1;
        return QPointF::dotProduct(sub, sub);
    };

    auto hasLinesIntesection = [&]( const QLineF& line1, const QLineF& line2) {
        QPointF intersectionPt;
        if( !line1.intersects(line2, &intersectionPt) ) {
            return false;
        }

        float dist1To1 = pointDistSq( intersectionPt, line1.p1());
        float dist1To2 = pointDistSq( intersectionPt, line1.p2());
        float line1Len = pointDistSq( line1.p1(), line1.p2());

        float dist2To1 = pointDistSq( intersectionPt, line2.p1());
        float dist2To2 = pointDistSq( intersectionPt, line2.p2());
        float line2Len = pointDistSq( line2.p1(), line2.p2());

        if( line1Len > dist1To1 &&  line1Len > dist1To2 &&
            line2Len > dist2To1 &&  line2Len > dist2To2 )  {
                return true;
            }

        return false;
    };

    // check the polygon border for interception
    // with other polygon borders
    QLineF lineToFirst(mFence.first(), pt);
    QLineF lineToLast(mFence.last(), pt);
    QPointF prevPt = mFence.first();
    for( auto i = 1; i < mFence.count(); i++ ) {
        QPointF nextPt = mFence[i];
        QLineF line(prevPt, nextPt);
        if( hasLinesIntesection( lineToFirst, line) ||
            hasLinesIntesection( lineToLast, line) ) {
                showPopup("Error:This border intersects other borders!");
                return false;
        }
        prevPt = nextPt;
    }

    mFence.append(pt);
    recreateFenceItem();
    return true;
}

void Scene::fenceRemove()
{
    if( mFence.count() ) {
        mFence.removeLast();
        recreateFenceItem();
    }
}

QVector<QGeoCoordinate> Scene::fenceGepPoints(Meadow* meadow)
{
    QVector<QGeoCoordinate> geoPoints;
    geoPoints.reserve(mFence.count());
    for( auto i = 0; i < mFence.count(); i ++) {
        geoPoints.append(meadow->getGeoLocation(mFence[i]));
    }

    return geoPoints;
}

bool Scene::saveFence(const QString &path)
{
    if( mFence.count() < 3) {
        qCritical() << "Fence points less then 3:" << path;
        return false;
    }

    QString content;
    bool isFirstPoint = true;
    for( QPointF p: mFence ) {
        if( !isFirstPoint ) {
            content.append(',');
        }
        content.append(QString("%1,").arg(p.x(), 0, 'f', 6));
        content.append(QString("%1").arg(p.y(), 0, 'f', 6));
        isFirstPoint = false;
    }

    if( !SimTools::fileWrite(path, content.toUtf8(), true)) {
        qCritical() << "Saving fence in %1 failed!" << path;
        return false;
    }

    return true;
}

bool Scene::loadFence(const QString &path)
{
    bool isOk = true;
    QByteArray ba = SimTools::fileRead(path, &isOk);

    if( !isOk) {
        qCritical() << "Loading fence from %1 failed!" << path;
        return false;
    }

    QString content = QString::fromUtf8(ba);
    QStringList valsList = content.split(',');
    if( valsList.count() % 2 ) {
        qCritical() << "Loading fence from %1 failed! Fence points %2 not even" << path << valsList.count();
        return false;
    }

    QVector<QString> valsVec = valsList.toVector();
    int pointsCount = valsVec.size() / 2;

    if( pointsCount < 3) {
        qCritical() << "Loading fence - points less then 3" << path;
        return false;
    }

    mFence.clear();
    mFence.reserve(pointsCount);

    for( auto i = 0; i < pointsCount; i ++) {
        int valsIndex = i * 2;
        float x = valsVec[valsIndex].toFloat();
        float y = valsVec[valsIndex+1].toFloat();
        mFence.append(QPointF(x, y));
    }

    recreateFenceItem();

    return true;
}

void Scene::showPopup(const QString &msg)
{
    if( mPopupLastMsg == msg) {
        mPopupLastMsgCount ++;
    }else {
        mPopupLastMsgCount = 1;
        mPopupLastMsg = msg;
    }
    mPopup->setTextColor(POPUP_TEXT_COLOR);
    mPopup->setBackColor(POPUP_BACK_COLOR);
    mPopup->setPlainText(mPopupLastMsgCount > 1 ? QString("%1 (%2)").arg(msg).arg(mPopupLastMsgCount): msg);
    mPopup->show();
    QPointF pos = mView->mapToScene(QPoint(0, 0));
    mPopup->setPos(pos);
    int duration = msg.length() * 60;
    if( duration < 1000 ) {
        duration = 1000;
    }

    mPopupTimer->start(duration);
}

void Scene::onPopupHide()
{
    mPopup->hide();
}


bool Scene::isPopup()
{
    return mPopup && mPopup->isVisible();
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
    mTextColor = INFO_TEXT_COLOR;
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
    setDefaultTextColor(mTextColor);
    painter->setBrush(mBackColor);
    painter->setPen(mTextColor);
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
