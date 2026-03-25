#include "luaman.h"
#include <QDebug>

extern "C" {
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
}

LuaMan::LuaMan(QObject *parent)
    : QObject{parent}
{}

void LuaMan::onThreadStarted()
{
}

void LuaMan::onThreadFinished()
{
}

void LuaMan::Thread::run() {
    mState = lua_open();

    lua_gc(mState, LUA_GCSTOP, 0);  // stop collector during initialization
    luaL_openlibs(mState);  // open libraries
    lua_gc(mState, LUA_GCRESTART, 0);

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

    registerVariables();
}

LuaMan::Thread::~Thread()
{
    lua_close(mState);
}

void LuaMan::Thread::onData(const QByteArray& data, const QString &profile)
{
    if (!mState) {
        qWarning() << "Lua state or data is null";
        return;
    }

    const QByteArray profileUtf8 = profile.toUtf8();

    lua_getglobal(mState, "onData");
    if (!lua_isfunction(mState, -1)) {
        lua_pop(mState, 1);
        qWarning() << "Lua global function onData is not defined";
        return;
    }

    // argument 1: profile name
    lua_pushstring(mState, profileUtf8.constData());

    // argument 2: parsed package table
    if (!pushUplinkPackage(data, profileUtf8)) {
        lua_pop(mState, 2); // function + profile arg
        qWarning() << "Failed to parse package for profile" << profile;
        return;
    }

    // call onData(name, package)
    const int status = lua_pcall(mState, 2, 0, 0);
    if (status) {
        qWarning() << "Lua onData error:" << lua_tostring(mState, -1);
        lua_pop(mState, 1);
        return;
    }
}

bool LuaMan::run(const QString &fileName) {
    Thread* t = new Thread(fileName, this);
    connect(t, &Thread::started, this, &LuaMan::onThreadStarted);
    connect(t, &Thread::finished, this, &LuaMan::onThreadFinished);
    t->start();
    return true;
}
