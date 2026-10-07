#ifndef DIALOGCOLLARSIM_H
#define DIALOGCOLLARSIM_H

#include <QDialog>
#include <QSettings>
#include <QSerialPort>

#define RESPONCE_COLOR_NORMAL QColor(0, 0, 0)
#define RESPONCE_COLOR_ERROR QColor(150, 0, 0)
#define RESPONCE_COLOR_WARNING QColor(150, 150, 0)
#define RESPONCE_COLOR_SUCESS QColor(0, 150, 0)
#define RESPONCE_COLOR_COMMAND QColor(128, 64, 128)
#define RESPONCE_COLOR_RESPONCE QColor(110, 99, 184)
#define RESPONCE_COLOR_INFO QColor(100, 100, 100)


class Animal;
class Collar;
class QSerialPort;

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

    bool mIsLoadingAnimals = false;
    QList<Animal*> mAnimals;
    Animal* mAnimal = nullptr;
    QSerialPort mPort;
    QSettings& mEnv;
    QIcon mIconMale, mIconFemale, mIconSoundOff, mIconSoundOn;
    Collar* mMirror = nullptr;
    void createMirror(const QString &animalName,
                      bool isMale,
                      const QString &euiHex,
                      const QString &akeyHex,
                      const QString &nkeyHex);
    void deleteMirror();

    void setLightsColor(const QColor& col);

    void closeEvent(QCloseEvent *e);

    void grabScreen();

    bool mIsLedOn = false;

    bool mIsOpeningPort = false;
    void sendToSerial(const QString &msg = QString(), bool isRequest = false);

    static uint32_t mBaudrates[];

    void addResponce(const QString& txt, const QColor& c = RESPONCE_COLOR_NORMAL);
    QMap<QString, int> mRequests;

    QStringList mCmds;

    void processSerialInput();

    QImage mImageDisconnected;



public:
    explicit DialogCollarSim(QSettings &env, QWidget *parent = nullptr);
    void loadAnimals(QList<Animal*> animals);
    ~DialogCollarSim();
    void sendScreen(const Animal *from);

    void update();

private slots:
    void on_comboAnimals_currentIndexChanged(int index);

    void on_btnGenArray_clicked();

    void on_btnMain_pressed();

    void on_editSerialCmd_returnPressed();

    void on_editSerialCmd_textChanged(const QString &arg1);

    void on_btnSend_clicked();

    void on_checkConnect_toggled(bool checked);

    void on_btnGenAKey_clicked();

    void on_btnGenNKey_clicked();

    void on_btnFlash_clicked();

    void on_serialPortError(QSerialPort::SerialPortError err);

    void on_btnGenEui_clicked();

    void on_btnClear_clicked();

    void on_btnReset_clicked();

    void on_btnCopyEui_clicked();

    void on_btnCopyAKey_clicked();

    void on_btnCopyNKey_clicked();

    void on_editEui_textChanged(const QString &arg1);

    void on_editAKey_textEdited(const QString &arg1);

    void on_editNKey_textChanged(const QString &arg1);

    void on_btnReload_clicked();

private:
    Ui::DialogCollarSim *ui;
};

#endif // DIALOGCOLLARSIM_H
