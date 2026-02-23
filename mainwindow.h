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

class MainWindow : public QMainWindow
{
    Q_OBJECT

    SimTools* mTools = nullptr;
    Scene* mScene = nullptr;
    Herd* mHerd = nullptr;
    Meadow* mMeadow = nullptr;
    QTimer mUpdateTimer;
    SceneView* mSceneView;
    QElapsedTimer mHerdTimer;
    QTimer* mReminder = nullptr;
    FocusAnim* mFocusAnim = nullptr;
    Network* mNetwork = nullptr;
    DevManager* mDevManager = nullptr;

    DialogConsole* mConsole = nullptr;

    void create(bool isLoad);

public:
    MainWindow(const QSettings &settings, QWidget *parent = nullptr);
    ~MainWindow();

    void onError(const QString &err);
    void setStatus(const QString& txt);
    inline DialogConsole* console(){ return mConsole; }

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
};

extern MainWindow* gMainWindow;
#endif // MAINWINDOW_H

