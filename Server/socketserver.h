// animal_server.cpp
// Build (Qt 6 example):
//   CMake: target_link_libraries(app Qt6::Core Qt6::Network)
//   qmake: QT += core network
// Run: ./app 9090 (or default port)

#include <QtCore>
#include <QtNetwork>

class SocketServer : public QObject {
    Q_OBJECT
public:
    // ---------- CONSTANTS ----------
    static constexpr quint16 DEFAULT_PORT = 8080;
    static constexpr int MAX_PENDING_CONNECTIONS = 256;
    static constexpr int READ_CHUNK_SIZE = 4096;

    explicit SocketServer(QObject* parent = nullptr);

    ~SocketServer() override {
        Stop();
    }

    bool Start(quint16 port = DEFAULT_PORT, const QHostAddress& bindAddress = QHostAddress::Any);

    void Stop();

    bool IsRunning() const { return mRunning; }
    int ClientCount() const { return mClientThreads.size(); }
    quint16 Port() const { return mPort; }
    QHostAddress BindAddress() const { return mBindAddress; }

private slots:
    void OnNewConnection();

private:
    // ---------- MEMBER VARIABLES ----------
    QTcpServer mServer;
    quint16 mPort;
    QHostAddress mBindAddress;
    bool mRunning;
    QHash<QTcpSocket*, QThread*> mClientThreads;
};
