#include <QCoreApplication>
#include "animalserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    quint16 port = AnimalServer::DEFAULT_PORT;
    if (app.arguments().size() > 1) {
        bool ok = false;
        int p = app.arguments().at(1).toInt(&ok);
        if (ok && p > 0 && p < 65536) port = static_cast<quint16>(p);
    }

    AnimalServer server;
    if (!server.Start(port, QHostAddress::Any)) {
        qCritical() << "Failed to start AnimalServer on port" << port;
        return 1;
    }

    qInfo() << "AnimalServer running. Press Ctrl+C to stop.";
    QObject::connect(&app, &QCoreApplication::aboutToQuit, &server, [&server]() { server.Stop(); });

    return app.exec();
}


