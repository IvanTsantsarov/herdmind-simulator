#ifndef DIALOGCOLLARSIM_H
#define DIALOGCOLLARSIM_H

#include <QDialog>
#include <QSettings>

class Animal;

namespace Ui {
class DialogCollarSim;
}

class ScreenWidget : public QWidget {
     QImage mImage;
protected:
     void paintEvent(QPaintEvent *event) override;
public:
     explicit ScreenWidget(QWidget *parent = nullptr) : QWidget(parent) { }
     void configImage(int cx, int cy, QColor c) {
        mImage = QImage(cx, cy, QImage::Format_RGB888);
        mImage.fill(c);
        update();
     }
     inline QImage& image(){ return mImage; }
};

class DialogCollarSim : public QDialog
{
    Q_OBJECT

    QList<Animal*> mAnimals;
    Animal* mAnimal = nullptr;
    QSettings& mEnv;
    QIcon mIconMale, mIconFemale, mIconSoundOff, mIconSoundOn;

    void setLightsColor(const QColor& col);

    void closeEvent(QCloseEvent *e);

    void grabScreen();

    bool mIsLedOn = false;

public:
    explicit DialogCollarSim(QSettings &env, QWidget *parent = nullptr);
    void init(QList<Animal*> animals);
    ~DialogCollarSim();
    void sendScreen(const Animal *from);

    void update(Animal *animal);

private slots:
    void on_comboAnimals_currentIndexChanged(int index);

    void on_btnGenArray_clicked();

    void on_btnMain_pressed();

private:
    Ui::DialogCollarSim *ui;
};

#endif // DIALOGCOLLARSIM_H
