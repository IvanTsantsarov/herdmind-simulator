#ifndef FOCUSANIM_H
#define FOCUSANIM_H

#include <QDialog>

class QPropertyAnimation;

namespace Ui {
class FocusAnim;
}

class FocusAnim : public QDialog
{
    Q_OBJECT

    bool mIsPlaying = false;
    QWidget* mTarget = NULL;

public:
    explicit FocusAnim(QWidget *parent = nullptr);
    ~FocusAnim();

    void start(QWidget* target);
    void stop();

    void refresh();

protected:
protected:
    void paintEvent(QPaintEvent *) override;

private:
    Ui::FocusAnim *ui;
    QPropertyAnimation *mAnimSize = NULL;
    QPropertyAnimation *mAnimFade = NULL;

};

#endif // FOCUSANIM_H
