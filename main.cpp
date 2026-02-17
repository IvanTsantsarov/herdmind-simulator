#include <QSettings>
#include <QApplication>
#include <QFile>

#include "mainwindow.h"
#include "simtools.h"

#define SETTINGS_NAME "settings.ini"

SimTools* gSimTools = nullptr;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString initPrint = QString("Gegga herdmind simulator\n"
                                "Qt version:%1")
                        .arg(qVersion());

    qDebug() << initPrint;

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
