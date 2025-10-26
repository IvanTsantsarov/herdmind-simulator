#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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

class MainWindow : public QMainWindow
{
    Q_OBJECT

    Scene* mScene = nullptr;
    Herd* mHerd = nullptr;
    Meadow* mMeadow = nullptr;
    QTimer mUpdateTimer;
    SceneView* mSceneView;
    QElapsedTimer mHerdTimer;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
private slots:

    void onUpdate();
    void on_btnGenerate_clicked();
    void onRowClicked(int row, int column);
    void on_checkShepard_toggled(bool checked);
    void on_checkParamsHerding_toggled(bool checked);
    void on_checkParamsG_toggled(bool checked);
};
#endif // MAINWINDOW_H
