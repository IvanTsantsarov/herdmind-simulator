#ifndef ANIMALSERVER_H
#define ANIMALSERVER_H

#include <QMap>
#include "socketserver.h"

class AnimalServer : public SocketServer
{

protected:
    void onEnter(QTcpSocket* socket);
    void onRead(QTcpSocket* socket);
    void onExit(QTcpSocket* socket);

public:
    explicit AnimalServer(QObject *parent = nullptr);

};

#endif // ANIMALSERVER_H
