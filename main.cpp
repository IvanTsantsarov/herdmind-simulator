#include <QSettings>
#include <QApplication>
#include <QFile>
#include <QDir>

#include "mainwindow.h"
#include "dialogconsole.h"
#include "simtools.h"

#include "defines.h"
#include "defines_settings.h"

SimTools* gSimTools = nullptr;


void gMessagehHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);
    if( !gMainWindow || !gMainWindow->console() ) {
        return;
    }

    DialogConsole* console = gMainWindow->console();

    switch (type) {
    case QtDebugMsg:
        if( console->showDebug() ) {
            console->debug(msg);
        }
        break;
    case QtWarningMsg:
        console->warning(msg);
        break;
    case QtCriticalMsg:
        console->error(msg);
        break;
    case QtInfoMsg:
        console->info(msg);
        break;
    case QtFatalMsg:
        console->fatal(msg);
        break;
    }
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(gMessagehHandler);

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
