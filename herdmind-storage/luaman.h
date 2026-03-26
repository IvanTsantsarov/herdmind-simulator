#ifndef LUAMAN_H
#define LUAMAN_H

#include <QObject>
#include <QSettings>
#include <QThread>
#include <QMap>

struct lua_State;

#define LUA_SCRIPTS_ROOT_DIR "lua"

class LuaMan : public QObject
{
    Q_OBJECT

public:
    explicit LuaMan(const QString& settingsPath, QObject *parent = nullptr);

    class Thread : QThread {
        friend class LuaMan;

        enum BinaryFormat
        {
            bf_int8  = 2,
            bf_uint8 = 3,
            bf_int16 = 4,
            bf_uint16 = 5,
            bf_int32 = 8,
            bf_uint32 = 9
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

        QMap<QString, BinaryFormatPackage> mProfileFormats; // <profileId, BinaryFormatPackage>

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

        inline LuaMan* luaMan(){ return (LuaMan*) parent(); }

        static Thread* self(lua_State* state);

        // uplink
        static int l_uplink_setFormat(lua_State* state);

        bool pushUplinkPackage(const QByteArray &data, const QString &profileName);

        // tools
        static int l_tools_geo2float(lua_State* state);
        static int l_tools_timestamp2string(lua_State* state);
    public:
        Thread(const QString& fileName, QObject* parent) : QThread(parent), mFilePath(QString(LUA_SCRIPTS_ROOT_DIR) + "/" + fileName) { }
        ~Thread();

        inline QString path(){ return mFilePath; }
        void onUplink(const QByteArray &data, const QString &profileId);

    };

    Thread* next();

private:


    QList<Thread*> mThreads;
    QList<Thread*>::iterator mCurrent;
    QString mTimeFormat;

    QMap<QString, QString> mProfilesNameId; // <profile name, profile id>
    QMap<QString, QString> mProfilesIdName; // <profile id, profile name>

protected slots:
    void onThreadStarted();
    void onThreadFinished();



public:
    bool run(const QString& fileName);
    QString getProfileName(const QString& profileId);
    QString getProfileId(const QString& profileName);
    const QString& timeFormat(){ return mTimeFormat; }

signals:
};

extern LuaMan* gLua;
#endif // LUAMAN_H
