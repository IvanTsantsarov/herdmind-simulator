#ifndef DIALOGCOLLARSIM_H
#define DIALOGCOLLARSIM_H

#include <QDialog>

class Animal;

namespace Ui {
class DialogCollarSim;
}

class DialogCollarSim : public QDialog
{
    Q_OBJECT

    QList<Animal*> mAnimals;
    Animal* mAnimal = nullptr;
    QImage mScreenImage;

public:
    explicit DialogCollarSim(QWidget *parent = nullptr);
    void init(QList<Animal*> animals);
    ~DialogCollarSim();
    void sendScreen(Animal* from);

private slots:
    void on_comboAnimals_currentIndexChanged(int index);

private:
    Ui::DialogCollarSim *ui;
};

#endif // DIALOGCOLLARSIM_H
