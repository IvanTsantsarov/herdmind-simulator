#ifndef LUAMAN_H
#define LUAMAN_H

#include <QObject>
#include <QThread>
#include <QMap>

struct lua_State;

#define LUA_SCRIPTS_ROOT_DIR "lua"

class LuaMan : public QObject
{
    Q_OBJECT

public:
    explicit LuaMan(QObject *parent = nullptr);

    class Thread : QThread {
        friend class LuaMan;

        enum BinaryFormat
        {
            bf_int8  = 1,
            bf_uint8 = 2,
            bf_int16 = 3,
            bf_uint16 = 4,
            bf_int32 = 5,
            bf_uint32 = 6
        };

        static inline int binaryFormatLen(BinaryFormat fmt) { return ((uint8_t)fmt) >> 1; }

        struct BinaryField
        {
            BinaryFormat format;
            QByteArray name;
        };

        struct BinaryFormatPackage {
            int byteLength;
            QList<BinaryField> fields;
        };

        QMap<QString, BinaryFormatPackage> mProfileFormats;

        int pushInt8( const QByteArray& data, int offset );
        int pushUint8( const QByteArray& data, int offset );
        int pushInt16( const QByteArray& data, int offset );
        int pushUint16( const QByteArray& data, int offset );
        int pushInt32( const QByteArray& data, int offset );
        int pushUint32( const QByteArray& data, int offset );


        QString mFilePath;

        lua_State* mState = nullptr;

        void registerVariables();

        void run() override;

        static Thread* self(lua_State* state);
        static int l_uplink_setFormat(lua_State* state);

        bool pushUplinkPackage(const QByteArray &data, const QString &profile);
    public:
        Thread(const QString& fileName, QObject* parent) : QThread(parent), mFilePath(QString(LUA_SCRIPTS_ROOT_DIR) + "/" + fileName) { }
        ~Thread();

        inline QString path(){ return mFilePath; }
        void onData(const QByteArray &data, const QString &profile);

    };

    Thread* next();

private:


    QList<Thread*> mThreads;
    QList<Thread*>::iterator mCurrent;

protected slots:
    void onThreadStarted();
    void onThreadFinished();



public:
    bool run(const QString& fileName);

signals:
};

extern LuaMan gLua;
#endif // LUAMAN_H
