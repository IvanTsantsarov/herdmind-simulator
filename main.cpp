#include <QSettings>
#include <QApplication>
#include <QFile>
#include <QDir>

#include "mainwindow.h"
#include "simtools.h"

#include "defines.h"

#define SETTINGS_NAME "settings.ini"

SimTools* gSimTools = nullptr;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qInfo() << "Gegga herdmind simulator v. " << VERSION;
    qInfo() << "by Ivan Tsantsarov " << 2025;
    qInfo() << "Qt version:" << qVersion();
    qInfo() << "Current dir:" << QDir::current().absolutePath();
    qInfo() << "Executable:" << QDir::currentPath();


    a.setWindowIcon(QIcon(":/gegga_logo.png"));

    if( !QFile::exists(SETTINGS_NAME) ) {
        QFile::copy("://" SETTINGS_NAME, SETTINGS_NAME);
    }

    Q_ASSERT( QFile::exists(SETTINGS_NAME) );

    QSettings settings(SETTINGS_NAME, QSettings::IniFormat);

    gSimTools = new SimTools(settings);

    MainWindow w(settings);
    w.show();
    return a.exec();
}
