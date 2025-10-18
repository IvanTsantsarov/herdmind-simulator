#include "socketserver.h"


SocketServer::SocketServer(QObject *parent)
    : QObject(parent),
    mPort(DEFAULT_PORT),
    mBindAddress(QHostAddress::Any),
    mRunning(false)
{
    connect(&mServer, &QTcpServer::newConnection,
            this, &SocketServer::OnNewConnection);
    mServer.setMaxPendingConnections(MAX_PENDING_CONNECTIONS);
}

bool SocketServer::Start(quint16 port, const QHostAddress &bindAddress) {
    if (mRunning) return true;

    mPort = port;
    mBindAddress = bindAddress;

    if (!mServer.listen(mBindAddress, mPort)) {
        qWarning() << "Listen failed:" << mServer.errorString();
        return false;
    }
    mRunning = true;
    qInfo() << "SocketServer listening on" << mBindAddress.toString() << ":" << mPort;
    return true;
}

void SocketServer::Stop() {
    if (!mRunning) return;
    mRunning = false;

    // Stop accepting new connections
    mServer.close();

    // Ask all clients to disconnect in their own threads
    const auto sockets = mClientThreads.keys();
    for (QTcpSocket* sock : sockets) {
        // Ensure the call runs in the socket's thread
        QMetaObject::invokeMethod(sock, [sock]() {
            if (sock->state() != QAbstractSocket::UnconnectedState)
                sock->disconnectFromHost();
        }, Qt::QueuedConnection);
    }

    // Wait for threads to finish (with timeout to avoid deadlocks)
    for (auto it = mClientThreads.begin(); it != mClientThreads.end(); /*in body*/) {
        QThread* th = it.value();
        // Give each thread a moment to exit cleanly
        if (!th->wait(2000)) {
            qWarning() << "Forcing thread quit for client";
            th->quit();
            th->wait(1000);
        }
        it = mClientThreads.erase(it);
        th->deleteLater();
    }

    qInfo() << "SocketServer stopped.";
}

void SocketServer::OnNewConnection() {
    while (mServer.hasPendingConnections()) {
        QTcpSocket* socket = mServer.nextPendingConnection();
        if (!socket) continue;

        // Create a dedicated thread for this client
        QThread* thread = new QThread();
        thread->setObjectName(QStringLiteral("ClientThread_%1").arg(reinterpret_cast<quintptr>(socket)));

        // Move the socket to the new thread
        socket->moveToThread(thread);

        // Wire up lifecycle and I/O (all run in the socket's thread)
        connect(thread, &QThread::started, socket, [socket]() {
            // Optional: per-socket initialization could go here
            // e.g., setSocketOption if needed
            Q_UNUSED(socket);
        });

        connect(socket, &QTcpSocket::readyRead, socket, [ socket]() {
            // --- Replace this with your application protocol handling ---
            QByteArray data = socket->read(READ_CHUNK_SIZE);
            if (!data.isEmpty()) {
                // Echo back
                qint64 written = socket->write(data);
                if (written < 0) {
                    qWarning() << "Write error:" << socket->errorString();
                    socket->disconnectFromHost();
                } else {
                    socket->flush();
                }
            }
        }, Qt::QueuedConnection);

        connect(socket, &QTcpSocket::disconnected, socket, [ socket]() {
            // Cleanup triggered from the socket's thread
            socket->deleteLater();
        }, Qt::QueuedConnection);

        // When socket is destroyed, stop and delete its thread
        connect(socket, &QObject::destroyed, thread, [ thread]() {
            thread->quit();
        }, Qt::QueuedConnection);

        // When thread finishes, remove bookkeeping and delete thread
        connect(thread, &QThread::finished, this, [this, socket, thread]() {
            mClientThreads.remove(socket);
            thread->deleteLater();
        });

        // Track pair and start thread
        mClientThreads.insert(socket, thread);
        thread->start();

        // Optional log
        qInfo() << "Client connected from"
                << socket->peerAddress().toString() << ":" << socket->peerPort();
    }
}
