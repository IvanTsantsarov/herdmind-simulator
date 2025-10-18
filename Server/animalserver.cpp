#include "animalserver.h"

void AnimalServer::onEnter(QTcpSocket *socket)
{

}

void AnimalServer::onRead(QTcpSocket *socket)
{

}

void AnimalServer::onExit(QTcpSocket *socket)
{

}

AnimalServer::AnimalServer(QObject *parent)
    : SocketServer{parent}
{

}
