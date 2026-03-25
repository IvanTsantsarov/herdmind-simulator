#include "luaman.h"
#include <QDebug>

extern "C" {
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
}

#define LUA_THREAD_THIS_GLOBAL "__thread_instance"

LuaMan::Thread *LuaMan::Thread::self(lua_State* state)
{
    lua_getglobal(state, LUA_THREAD_THIS_GLOBAL);
    Thread* instance = static_cast<Thread*>(lua_touserdata(state, -1));
    lua_pop(state, 1);
    return instance;
}


int LuaMan::Thread::pushInt8(const QByteArray &data, int offset)
{
    int8_t val = data[offset];
    lua_pushinteger(mState, val);
    return 1;
}

int LuaMan::Thread::pushUint8(const QByteArray &data, int offset)
{
    uint8_t val = data[offset];
    lua_pushinteger(mState, val);
    return 1;
}

int LuaMan::Thread::pushInt16(const QByteArray &data, int offset)
{
    int16_t val = (((int16_t)data[offset]) << 8) | (int16_t)data[offset + 1];
    lua_pushinteger(mState, val);
    return 2;
}

int LuaMan::Thread::pushUint16(const QByteArray &data, int offset)
{
    uint16_t val = (((uint16_t)data[offset]) << 8) | (uint16_t)data[offset + 1];
    lua_pushinteger(mState, val);
    return 2;
}

int LuaMan::Thread::pushInt32(const QByteArray &data, int offset)
{
    int32_t val =   (((int32_t)data[offset]) << 24)
                  | (((int32_t)data[offset + 1]) << 16)
                  | (((int32_t)data[offset + 2]) << 8)
                  | (((int32_t)data[offset + 3]));
    lua_pushinteger(mState, val);
    return 4;
}

int LuaMan::Thread::pushUint32(const QByteArray &data, int offset)
{
    uint32_t val =    (((uint32_t)data[offset]) << 24)
                    | (((uint32_t)data[offset + 1]) << 16)
                    | (((uint32_t)data[offset + 2]) << 8)
                    | (((uint32_t)data[offset + 3]));
    lua_pushinteger(mState, val);
    return 4;
}

void LuaMan::Thread::registerVariables()
{
    lua_pushlightuserdata(mState, this);
    lua_setglobal(mState, LUA_THREAD_THIS_GLOBAL);
    //
    // 1) global table: format
    //
    lua_newtable(mState);

    lua_pushinteger(mState, bf_int8);
    lua_setfield(mState, -2, "int8");

    lua_pushinteger(mState, bf_uint8);
    lua_setfield(mState, -2, "uint8");

    lua_pushinteger(mState, bf_int16);
    lua_setfield(mState, -2, "int16");

    lua_pushinteger(mState, bf_uint16);
    lua_setfield(mState, -2, "uint16");

    lua_pushinteger(mState, bf_int32);
    lua_setfield(mState, -2, "int32");

    lua_pushinteger(mState, bf_uint32);
    lua_setfield(mState, -2, "uint32");

    lua_setglobal(mState, "format");

    //
    // 2) global table: uplink
    //
    lua_newtable(mState);

    lua_pushcfunction(mState, LuaMan::Thread::l_uplink_setFormat);
    lua_setfield(mState, -2, "setFormat");

    // lua_pushcfunction(mState, LuaMan::Thread::l_uplink_data);
    // lua_setfield(mState, -2, "data");

    lua_setglobal(mState, "uplink");
}

int LuaMan::Thread::l_uplink_setFormat(lua_State *state)
{
    luaL_checktype(state, 1, LUA_TTABLE);
    const char* profile = luaL_checkstring(state, 2);

    LuaMan::Thread* thread = self(state);
    if (!thread) {
        return luaL_error(state, "C++ thread instance is null");
    }

    const QByteArray profileKey(profile);
    QList<BinaryField> fields;

    const int count = static_cast<int>(lua_objlen(state, 1));

    for (int i = 1; i <= count; ++i) {
        // schema[i]
        lua_rawgeti(state, 1, i);

        if (!lua_istable(state, -1)) {
            lua_pop(state, 1);
            return luaL_error(state, "uplink.setFormat: element %d must be a table", i);
        }

        // schema[i][1] -> field name
        lua_rawgeti(state, -1, 1);
        const char* fieldName = luaL_checkstring(state, -1);
        lua_pop(state, 1);

        // schema[i][2] -> field type
        lua_rawgeti(state, -1, 2);
        if (!lua_isnumber(state, -1)) {
            lua_pop(state, 2); // pop type and schema[i]
            return luaL_error(state, "uplink.setFormat: element %d type must be number", i);
        }

        BinaryField field;
        field.name = QByteArray(fieldName);
        field.format = static_cast<BinaryFormat>(lua_tointeger(state, -1));
        fields.append(field);

        lua_pop(state, 1); // pop type
        lua_pop(state, 1); // pop schema[i]
    }

    BinaryFormatPackage bfPackage;
    bfPackage.byteLength = 0;
    bfPackage.fields = fields;
    for( const BinaryField& field : bfPackage.fields ) {
        bfPackage.byteLength += binaryFormatLen(field.format);
    }

    thread->mProfileFormats[profileKey] = bfPackage;

    qDebug() << "Stored format for profile" << profileKey;

    return 0;
}

bool LuaMan::Thread::pushUplinkPackage(const QByteArray &data, const QString &profile)
{
    if (!mProfileFormats.contains(profile)) {
        qWarning() << "Unknown profile:" << profile;
        return false;
    }

    BinaryFormatPackage format = mProfileFormats[profile];
    if( data.length() != format.byteLength ) {
        qWarning() << "Profile" << profile << "lenght missmatch " << data.length() << " vs " << format.byteLength;
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
