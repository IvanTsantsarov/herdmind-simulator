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
    // ApiRest* mApiRest = nullptr;

    void setStatus(const QString& txt);

public:
    MainWindow(const QSettings &settings, QWidget *parent = nullptr);
    ~MainWindow();

    void onError(const QString &err);

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
};
#endif // MAINWINDOW_H
