#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QElapsedTimer>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class Scene;
class SceneView;
class Herd;
class Meadow;
class FocusAnim;
class Network;
class SimTools;
class ApiRest;
class DevManager;
class DialogConsole;
class DialogDeviceMsg;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    SimTools* mTools = nullptr;
    Scene* mScene = nullptr;
    Herd* mHerd = nullptr;
    Meadow* mMeadow = nullptr;
    QTimer mUpdateTimer;
    SceneView* mSceneView;
    QTimer* mReminder = nullptr;
    FocusAnim* mFocusAnim = nullptr;
    Network* mNetwork = nullptr;
    DevManager* mDevManager = nullptr;

    DialogConsole* mConsole = nullptr;
    DialogDeviceMsg* mDevMsg = nullptr;

    QSettings& mEnv;
    const QSettings &mSettings;

    void create(bool isLoad);

    void closeEvent(QCloseEvent* e);

public:
    MainWindow(QSettings &env, const QSettings &settings, QWidget *parent = nullptr);
    ~MainWindow();

    inline Herd* herd(){ return mHerd; }
    inline Meadow* meadow(){ return mMeadow; }
    inline DevManager* devManager(){ return mDevManager; }
    inline Network* network(){ return mNetwork; }
    inline SimTools* tools(){ return mTools; }

    void onError(const QString &err);
    void onConsoleClose();
    void onDeviceMsgClose();
    void setStatus(const QString& txt);
    void onDeviceMessage(const QString& devEUI, const QJsonObject &jobjResponse);
    void onDevicesReady(bool isStore);
    inline DialogConsole* console(){ return mConsole; }
    void errorMsgBox(const QString& msg);

protected:
    void moveEvent(QMoveEvent *);
    void resizeEvent(QResizeEvent *);
private:
    Ui::MainWindow *ui;
private slots:

    void onUpdate();
    void on_btnGenerate_clicked();
    void onRowClicked(int row, int column);
    void on_checkShepard_toggled(bool checked);
    void on_checkParamsHerding_toggled(bool checked);
    void on_checkParamsG_toggled(bool checked);
    void onConnectReminger();
    void on_btnLoad_clicked();
    void on_actionConsole_toggled(bool arg1);
    void on_actionDeviceMsg_toggled(bool arg1);
    void on_btnRefill_clicked();
    void on_checkGrowingMeadow_toggled(bool checked);
    void on_btnShowInfo_toggled(bool checked);
};

extern MainWindow* gMainWindow;
#endif // MAINWINDOW_H

