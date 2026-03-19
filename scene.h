#ifndef SCENE_H
#define SCENE_H


#include <QObject>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsPolygonItem>

class QStyleOptionGraphicsItem;
class QGraphicsItem;

class Herd;
class Meadow;
class Animal;
class Scene;
class SceneView;
class Gateway;
class Network;

class SelectableItem : public QObject, public QGraphicsPolygonItem {
    Q_OBJECT

    float mScale = 0.0;
public:
    SelectableItem( const QPolygonF& poly);
    SelectableItem( float radius );

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void startPulseAnimation();
    inline Scene* parentScene() { return reinterpret_cast<Scene*>(scene()); }
protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &v) override;
    virtual void onSelection() = 0;
};



class AnimalItem : public SelectableItem {
    Q_OBJECT
    Animal* mAnimal = nullptr;
protected:
    void onSelection();
public:
    AnimalItem( Animal* animal, const QPolygonF& poly) : SelectableItem(poly), mAnimal(animal) { }
    Animal* animal(){ return mAnimal; }
};

class GatewayItem : public SelectableItem {
    Q_OBJECT
    Gateway* mGateway = nullptr;
protected:
    void onSelection();
public:
    GatewayItem( Gateway* gw, const QPolygonF& poly) : SelectableItem(poly), mGateway(gw) { }
    Gateway* gateway(){ return mGateway; }
};


class TextItem : public QGraphicsTextItem
{
    QColor mBackColor;
public:
    TextItem(const QString &text, Scene* scene);

    void setBackColor(const QColor& c){ mBackColor = c; }

    void paint( QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *w);
};



class Scene : public QGraphicsScene
{
    Q_OBJECT

    QVector<AnimalItem*> mAnimalItems;
    AnimalItem* mAnimalItemSelected = nullptr;
    QVector<QGraphicsLineItem*> mLinesAnimals;

    QVector<GatewayItem*> mGatewayItems;
    GatewayItem* mGatewayItemSelected = nullptr;
    QVector<QGraphicsLineItem*> mLinesGateways;

    QGraphicsEllipseItem* mAttractor = nullptr;
    QVector<QGraphicsRectItem*> mLawns;

    TextItem* mItemInfo = nullptr;
    TextItem* mCursorInfo = nullptr;

    SceneView* mView = nullptr;

    void clear();
public:
    explicit Scene(QObject *parent = nullptr);

    void create(SceneView *view, Herd *herd, Network* network, int lawnsCount, int collarPairsCount, int gatewayPairsCount);
    void update(Herd* herd, Meadow *meadow, Network *network, bool isInitial = false, float diameter = 0.0f);

    void selectAnimalItem(int index);
    void selectAnimalItem(AnimalItem* item);

    void selectGatewayItem(GatewayItem* item);

    inline AnimalItem* selectedAnimal(){ return mAnimalItemSelected; }
    void setCursorInfoPos(const QPointF& pt, float kg);

public slots:
    void onFigurePick(QGraphicsPolygonItem* item, QPointF pos);
    void onFigureMove(QGraphicsPolygonItem *item, QPointF pos);
    void onFigureDrop(QGraphicsPolygonItem *item, QPointF pos);

};

#endif // SCENE_H
