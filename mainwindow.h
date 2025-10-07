#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class Scene;
class SceneView;
class Herd;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    Scene* mScene;
    Herd* mHerd;
    QTimer mUpdateTimer;
    SceneView* mSceneView;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
private slots:

    void onUpdate();
};
#endif // MAINWINDOW_H
