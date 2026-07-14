#include <QPropertyAnimation>
#include <QPainter>
#include "focusanim.h"
#include "ui_focusanim.h"


#define FOCUS_ANIM_DURATION 400
#define FOCUS_ANIM_LOOPS 3
#define FOCUS_RECT_TICKNESS 5
#define FOCUS_RECT_COLOR QColor(100, 100, 100, 160)
#define FOCUS_ANIM_GROW QPoint(20, 20)
#define FOCUS_ANIM_CURVE QEasingCurve::OutQuint

FocusAnim::FocusAnim(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FocusAnim)
{
    ui->setupUi(this);

    setWindowFlags( Qt::Tool | Qt::FramelessWindowHint );
    // setAttribute(Qt::WA_TranslucentBackground);
}

FocusAnim::~FocusAnim()
{
    delete ui;
}

void FocusAnim::start(QWidget *target)
{
    raise();
    show();

    mTarget = target;
    double ratio = target->devicePixelRatioF();

    QRect r = target->rect();
    QPoint p1 = target->mapToGlobal( r.topLeft() )  * ratio ;
    QPoint p2 = target->mapToGlobal( r.bottomRight() )  * ratio ;


    if( mAnimSize ) {
        delete mAnimSize;
    }

    if( mAnimFade ) {
        delete mAnimFade;
    }

    mAnimSize = new QPropertyAnimation( this ,"geometry" );
    mAnimSize->setStartValue( QRect(p1, p2) );
    mAnimSize->setEndValue( QRect( p1 - FOCUS_ANIM_GROW,  p2 + FOCUS_ANIM_GROW) );
    mAnimSize->setEasingCurve(FOCUS_ANIM_CURVE);
    mAnimSize->setDuration( FOCUS_ANIM_DURATION );
    mAnimSize->setLoopCount( FOCUS_ANIM_LOOPS );
    mAnimSize->start();

    QString platform = QGuiApplication::platformName();

    if (platform == "windows" || platform == "cocoa" || platform == "xcb") {
        mAnimFade = new QPropertyAnimation( this ,"windowOpacity" );
        mAnimFade->setStartValue( 1.0f );
        mAnimFade->setEndValue( 0.0f );
        mAnimFade->setEasingCurve(FOCUS_ANIM_CURVE);
        mAnimFade->setDuration( FOCUS_ANIM_DURATION );
        mAnimFade->setLoopCount( FOCUS_ANIM_LOOPS );
        mAnimFade->start();
    }

    mIsPlaying = true;

    connect( mAnimSize, &QPropertyAnimation::finished, this, [this](){ mIsPlaying = false; hide(); } );
}


void FocusAnim::stop()
{
    if( mAnimSize) {
        mAnimSize->stop();
    }

    if( mAnimFade ) {
        mAnimFade->stop();
    }

    hide();
}

void FocusAnim::refresh()
{
    if( !mIsPlaying ) {
        return;
    }

}

void FocusAnim::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Draw semi-transparent outer rectangle

    p.setBrush(FOCUS_RECT_COLOR);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(rect(), 10, 10 );

    // Cut out inner transparent hole
    QRect r = rect();
    QRegion outer(r);
    QRegion inner(r.adjusted(FOCUS_RECT_TICKNESS, FOCUS_RECT_TICKNESS, -FOCUS_RECT_TICKNESS, -FOCUS_RECT_TICKNESS), QRegion::Rectangle);
    QRegion hollow = outer.subtracted(inner);
    setMask(hollow);
}

