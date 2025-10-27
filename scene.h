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

class AnimalItem : public QObject, public QGraphicsPolygonItem {
    Q_OBJECT

    float mScale = 0.0;
    Animal* mAnimal = nullptr;
public:
    AnimalItem( Animal* animal, const QPolygonF& poly) : QGraphicsPolygonItem(poly), mAnimal(animal) {}
    AnimalItem( Animal* animal, const QRectF &rect, QGraphicsItem *parent = nullptr)
        : QGraphicsPolygonItem(rect, parent), mAnimal(animal) {}

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void startPulseAnimation();
    Animal* animal(){ return mAnimal; }
    Scene* animalScene();
protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &v) override;
};

inline Scene *AnimalItem::animalScene()
{
    return reinterpret_cast<Scene*>(scene());
}


class TextItem : public QGraphicsTextItem
{
    QColor mBackColor;
public:
    TextItem(const QString &text) :
        QGraphicsTextItem(text) { }

    void setBackColor(const QColor& c){ mBackColor = c; }

    void paint( QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *w);
};



class Scene : public QGraphicsScene
{
    Q_OBJECT


    QVector<AnimalItem*> mItems;
    QVector<QGraphicsLineItem*> mLines;
    AnimalItem* mItemSelected = nullptr;

    QGraphicsEllipseItem* mAttractor = nullptr;
    QVector<QGraphicsRectItem*> mLawns;

    TextItem* mItemInfo = nullptr;

    void clear();
public:
    explicit Scene(QObject *parent = nullptr);

    void create(Herd *herd, int animalsCount, int pairsCount, int lawnsCount);
    void update(Herd* herd, Meadow *meadow, bool isInitial = false, float diameter = 0.0f);

    void selectAnimalItem(int index);
    void selectAnimalItem(AnimalItem* item);

    inline AnimalItem* selectedAnimal(){ return mItemSelected; }

public slots:
    void onFigurePick(QGraphicsPolygonItem* item, QPointF pos);
    void onFigureMove(QGraphicsPolygonItem *item, QPointF pos);
    void onFigureDrop(QGraphicsPolygonItem *item, QPointF pos);

};

#endif // SCENE_H
