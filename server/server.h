// animal_server.cpp
// Build (Qt 6 example):
//   CMake: target_link_libraries(app Qt6::Core Qt6::Network)
//   qmake: QT += core network
// Run: ./app 9090 (or default port)

#include <QtCore>
#include <QtNetwork>

class AnimalServer : public QObject {
    Q_OBJECT
public:
    // ---------- CONSTANTS ----------
    static constexpr quint16 DEFAULT_PORT = 8080;
    static constexpr int MAX_PENDING_CONNECTIONS = 256;
    static constexpr int READ_CHUNK_SIZE = 4096;

    explicit AnimalServer(QObject* parent = nullptr)
        : QObject(parent),
        mPort(DEFAULT_PORT),
        mBindAddress(QHostAddress::Any),
        mRunning(false)
    {
        connect(&mServer, &QTcpServer::newConnection,
                this, &AnimalServer::OnNewConnection);
        mServer.setMaxPendingConnections(MAX_PENDING_CONNECTIONS);
    }

    ~AnimalServer() override {
        Stop();
    }

    bool Start(quint16 port = DEFAULT_PORT, const QHostAddress& bindAddress = QHostAddress::Any) {
        if (mRunning) return true;

        mPort = port;
        mBindAddress = bindAddress;

        if (!mServer.listen(mBindAddress, mPort)) {
            qWarning() << "Listen failed:" << mServer.errorString();
            return false;
        }
        mRunning = true;
        qInfo() << "AnimalServer listening on" << mBindAddress.toString() << ":" << mPort;
        return true;
    }

    void Stop() {
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

        qInfo() << "AnimalServer stopped.";
    }

    bool IsRunning() const { return mRunning; }
    int ClientCount() const { return mClientThreads.size(); }
    quint16 Port() const { return mPort; }
    QHostAddress BindAddress() const { return mBindAddress; }

private slots:
    void OnNewConnection() {
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

            connect(socket, &QTcpSocket::readyRead, socket, [this, socket]() {
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

            connect(socket, &QTcpSocket::disconnected, socket, [this, socket]() {
                // Cleanup triggered from the socket's thread
                socket->deleteLater();
            }, Qt::QueuedConnection);

            // When socket is destroyed, stop and delete its thread
            connect(socket, &QObject::destroyed, thread, [this, thread]() {
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

private:
    // ---------- MEMBER VARIABLES ----------
    QTcpServer mServer;
    quint16 mPort;
    QHostAddress mBindAddress;
    bool mRunning;
    QHash<QTcpSocket*, QThread*> mClientThreads;
};

// ----------------- Minimal example main() -----------------
int main(int argc, char** argv) {
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

#include "animal_server.moc"
