#include <QSettings>
#include <QFile>
#include <QCoreApplication>
#include "luaman.h"
#include "storage.h"
#include "defines.h"
#include "tools.h"

#include <iostream>
using namespace std;

LuaMan gLua;


void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);
    QByteArray localMsg = msg.toLocal8Bit();

    const char* color = "\033[0m"; // default

    switch (type) {
    case QtDebugMsg:
        color = "\033[36m"; // cyan
        break;
    case QtInfoMsg:
        color = "\033[32m"; // green
        break;
    case QtWarningMsg:
        color = "\033[33m"; // yellow
        break;
    case QtCriticalMsg:
        color = "\033[47;31m"; // red
        break;
    case QtFatalMsg:
        color = "\033[47;35m"; // magenta
        break;
    }

    fprintf(stderr, "%s%s\033[0m\n", color, localMsg.constData());

    if (type == QtFatalMsg) {
        abort();
    }
}


int main(int argc, char *argv[])
{
    qInstallMessageHandler(messageHandler);

    QCoreApplication a(argc, argv);

    // a.setWindowIcon(QIcon(":/gegga_logo.png"));

    if( !Tools::fileRestoreResources(SETTINGS_NAME) ) {
        return 1;
    }

    if( !Tools::fileRestoreResources(SETTINGS_NAME_EXTERNAL) ) {
        return 2;
    }

    char choice;
    printf("Load external settins (to connect later to remote server)? [y/n]");
    flush(cout);
    a.processEvents();
    cin >> choice;
    a.processEvents();
    QString settingsName = choice =='y' || choice =='Y' ? SETTINGS_NAME_EXTERNAL : SETTINGS_NAME;

    QSettings settings(settingsName, QSettings::IniFormat);

    Storage s(settings);

    s.run();



    return a.exec();
}
