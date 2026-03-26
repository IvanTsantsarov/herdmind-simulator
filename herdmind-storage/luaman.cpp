#include "luaman.h"
#include <QDebug>

extern "C" {
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
}

#define LUA_GLOBAL_ON_UPLINK "onUplink"

LuaMan::LuaMan(const QString &settingsPath, QObject *parent)
    : QObject{parent}
{
    QSettings settings(settingsPath, QSettings::IniFormat);

    mTimeFormat = settings.value("Lua/TimeFormat").toString();

    settings.beginGroup("Profiles");

    QStringList keys = settings.childKeys();
    for (const QString& key : keys) {
        QString value = settings.value(key).toString();
        mProfilesNameId[key] = value;
        mProfilesIdName[value] = key;
    }

    settings.endGroup();
}

LuaMan::Thread *LuaMan::next()
{
    if( !mThreads.count() ) {
        return nullptr;
    }

    Thread* t = *mCurrent;
    mCurrent ++;

    if(mCurrent == mThreads.end()) {
        mCurrent = mThreads.begin();
    }

    return t;
}

void LuaMan::onThreadStarted()
{
    Thread* t = (Thread*) sender();
    qInfo() << "Lua thread started" << t->path();
}

void LuaMan::onThreadFinished()
{
    Thread* t = (Thread*) sender();

    if( t == *mCurrent ) {
        next();
    }

    mThreads.removeOne(t);

}

void LuaMan::Thread::run() {
    mState = luaL_newstate();

    lua_gc(mState, LUA_GCSTOP, 0);  // stop collector during initialization
    luaL_openlibs(mState);  // open libraries
    lua_gc(mState, LUA_GCRESTART, 0);

    // register all tables and variables
    registerVariables();

    // Load the script file first
    int status = luaL_loadfile(mState, mFilePath.toUtf8().constData());
    if (status) {
        const char* err = lua_tostring(mState, -1);
        qCritical() << "Lua load error:" << mFilePath << (err ? err : "unknown error");
        lua_pop(mState, 1); // remove error message from stack
        return;
    }

    // Run the loaded chunk in protected mode
    status = lua_pcall(mState, 0, 0, 0);
    if (status) {
        const char* err = lua_tostring(mState, -1);
        qCritical() << "Lua runtime error:" << mFilePath << (err ? err : "unknown error");
        lua_pop(mState, 1); // remove error message from stack
        return;
    }

    exec();

    lua_close(mState);
    mState = nullptr;
}

LuaMan::Thread::~Thread()
{

}

bool LuaMan::Thread::pushUplinkPackage(const QByteArray &data, const QString &profileName)
{
    if (!mProfileFormats.contains(profileName)) {
        qWarning() << "Unknown profile:" << profileName;
        return false;
    }

    BinaryFormatPackage& format = mProfileFormats[profileName];
    if( data.length() != format.byteLength ) {
        qWarning() << "Profile" << profileName << "lenght missmatch " << data.length() << " vs " << format.byteLength;
        return false;
    }

    int offset = 0;

    lua_newtable(mState);

    for (const BinaryField& field : format.fields) {
        lua_pushstring(mState, field.name.constData());

        switch (field.format) {
        case bf_int8: offset += pushInt8(data, offset); break;
        case bf_uint8: offset += pushUint8(data, offset); break;
        case bf_int16: offset += pushInt16(data, offset); break;
        case bf_uint16: offset += pushUint16(data, offset); break;
        case bf_int32: offset += pushInt32(data, offset); break;
        case bf_uint32: offset += pushUint32(data, offset); break;
        default:
            lua_pop(mState, 2);
            qWarning() << "Unsupported field type" << field.format << "for" << field.name;
            return false;
        }

        lua_settable(mState, -3);
    }

    return true;
}



void LuaMan::Thread::onUplink(const QByteArray& data, const QString &profileId)
{
    if (!mState) {
        qWarning() << "Lua state is null";
        return;
    }

    lua_getglobal(mState, LUA_GLOBAL_ON_UPLINK);
    if (!lua_isfunction(mState, -1)) {
        lua_pop(mState, 1);
        qWarning() << "Lua global function onUplink is not defined";
        return;
    }

    QString profileName = luaMan()->getProfileName(profileId);
    if (profileName.isEmpty()) {
        lua_pop(mState, 1); // function
        qCritical() << "Profile name missing for this id:" << profileId;
        return;
    }

    if (!pushUplinkPackage(data, profileName)) {
        lua_pop(mState, 1); // function only
        qWarning() << "Failed to parse package for profile" << profileName;
        return;
    }

    QByteArray profileNameUtf8 = profileName.toUtf8();
    lua_pushstring(mState, profileNameUtf8.constData());

    const int status = lua_pcall(mState, 2, 0, 0);
    if (status) {
        qWarning() << "Lua onUplink error:" << lua_tostring(mState, -1);
        lua_pop(mState, 1);
    }
}

bool LuaMan::run(const QString &fileName) {
    Thread* t = new Thread(fileName, this);
    connect(t, &Thread::started, this, &LuaMan::onThreadStarted);
    connect(t, &Thread::finished, this, &LuaMan::onThreadFinished);
    t->start();

    mThreads.append(t);
    mCurrent = mThreads.begin();
    return true;
}

QString LuaMan::getProfileName(const QString &profileId)
{
    if( mProfilesIdName.contains(profileId)) {
        return mProfilesIdName[profileId];
    }

    return QString();
}

QString LuaMan::getProfileId(const QString &profileName)
{
    if( mProfilesNameId.contains(profileName)) {
        return mProfilesNameId[profileName];
    }

    return QString();
}

