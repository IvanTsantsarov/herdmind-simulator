#include <QSettings>
#include <QMessageBox>
#include <QApplication>
#include <QFile>
#include <QDir>

#include "simtimer.h"
#include "mainwindow.h"
#include "dialogconsole.h"
#include "simtools.h"

#include "defines.h"
#include "defines_settings.h"

#define ENVIRONMENT_INI "environment.ini"

SimTimer gSimTimerObject;
SimTools* gSimTools = nullptr;

QtMessageHandler gOoriginalHandler = nullptr;

void gMessagehHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);
    if( !gMainWindow || !gMainWindow->console() ) {
        if( gOoriginalHandler ) {
            gOoriginalHandler(type, context, msg);
        }
        return;
    }

    DialogConsole* console = gMainWindow->console();

    switch (type) {
    case QtDebugMsg:
        if( console->showDebug() ) {
            console->debug(msg);
        }
        break;
    case QtInfoMsg:
        console->info(msg);
        break;
    case QtWarningMsg:
        console->warning(msg);
        break;
    case QtCriticalMsg:
        console->error(msg);
        gMainWindow->setStatus( QString("Critical:%1").arg(msg) );
        // gMainWindow->errorMsgBox(msg);

        break;
    case QtFatalMsg:
        console->fatal(msg);
        gMainWindow->errorMsgBox(msg);
        break;
    }
}

int main(int argc, char *argv[])
{
    gOoriginalHandler = qInstallMessageHandler(gMessagehHandler);

    // restore environemnt
    if( !SimTools::fileExists(ENVIRONMENT_INI) ) {
        if( !SimTools::fileRestoreResources(ENVIRONMENT_INI) ) {
            qWarning() << "Cannot restore file" << ENVIRONMENT_INI;
        }
    }


    QApplication a(argc, argv);

    qInfo() << "Gegga herdmind simulator v. " << VERSION;
    qInfo() << "by Ivan Tsantsarov " << 2025;
    qInfo() << "Qt version:" << qVersion();
    qInfo() << "Current dir:" << QDir::current().absolutePath();
    qInfo() << "Executable:" << QDir::currentPath();

    a.setWindowIcon(QIcon(":/gegga_logo.png"));

    if( !SimTools::fileRestoreResources(SETTINGS_NAME) ) {
        return 1;
    }

    if( !SimTools::fileRestoreResources(SETTINGS_NAME_EXTERNAL) ) {
        return 2;
    }

    QString settingsName;

    QSettings env(ENVIRONMENT_INI);
    bool isRemoteSession = env.value("Main/Remote").toBool();

    switch( QMessageBox::question(nullptr, "Herdming Simulator",
                                  "Connect to external Chirpstack?",
                                  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                  isRemoteSession ? QMessageBox::Yes : QMessageBox::No ) )
    {
    case QMessageBox::Yes:
        settingsName = SETTINGS_NAME_EXTERNAL;
        isRemoteSession = true;
        break;
    case QMessageBox::No:
        settingsName = SETTINGS_NAME;
        isRemoteSession = false;
        break;
    default:
        return 3;
    }

    // store choice
    env.setValue("Main/Remote", isRemoteSession);

    Q_ASSERT( QFile::exists(settingsName) );

    // TODO: Messagebox to ask external
    QSettings settings(settingsName, QSettings::IniFormat);

    gSimTools = new SimTools(settings);

    MainWindow w(env,settings);
    w.show();
    return a.exec();
}
