#ifndef DIALOGINITIAL_H
#define DIALOGINITIAL_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class DialogInitial;
}

class DialogInitial : public QDialog
{
    Q_OBJECT

    QSettings &mEnv;
    bool mIsOk = false;
public:
    explicit DialogInitial(QSettings &env, QWidget *parent = nullptr);
    ~DialogInitial();

    bool isLocal();
    bool isRemote();
    bool isSimulation();
    bool isOk() { return mIsOk; }

private slots:
    void on_buttonBox_accepted();

private:
    Ui::DialogInitial *ui;
};

#endif // DIALOGINITIAL_H
