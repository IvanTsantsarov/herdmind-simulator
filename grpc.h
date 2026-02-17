#ifndef GRPC_H
#define GRPC_H

#include <QObject>

class gRpc : public QObject
{
    Q_OBJECT
public:
    explicit gRpc(QObject *parent = nullptr);

signals:
};

#endif // GRPC_H
