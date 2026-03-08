#include <QSettings>
#include <QFile>
#include <QCoreApplication>
#include "storage.h"
#include "defines.h"

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
        color = "\033[31m"; // red
        break;
    case QtFatalMsg:
        color = "\033[35m"; // magenta
        break;
    }

    fprintf(stderr, "%s%s\033[0m\n", color, localMsg.constData());

    if (type == QtFatalMsg)
        abort();
}


int main(int argc, char *argv[])
{
    qInstallMessageHandler(messageHandler);

    QCoreApplication a(argc, argv);

    // a.setWindowIcon(QIcon(":/gegga_logo.png"));

    if( !QFile::exists(SETTINGS_NAME) ) {
        QFile::copy("://" SETTINGS_NAME, SETTINGS_NAME);
    }

    Q_ASSERT( QFile::exists(SETTINGS_NAME) );

    QSettings settings(SETTINGS_NAME, QSettings::IniFormat);

    Storage s(settings);

    s.run();

    return a.exec();
}
