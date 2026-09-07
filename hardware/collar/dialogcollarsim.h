#ifndef DIALOGCOLLARSIM_H
#define DIALOGCOLLARSIM_H

#include <QDialog>

namespace Ui {
class DialogCollarSim;
}

class DialogCollarSim : public QDialog
{
    Q_OBJECT

public:
    explicit DialogCollarSim(QWidget *parent = nullptr);
    ~DialogCollarSim();

private:
    Ui::DialogCollarSim *ui;
};

#endif // DIALOGCOLLARSIM_H
