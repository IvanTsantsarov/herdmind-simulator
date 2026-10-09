#include "memory.h"
#include "defines.h"

#ifndef ONPC
    #include <Preferences.h>
#endif

Preferences gMem;

#define MEMORY_SECTION "Main"

Memory::Memory()
{
}

size_t Memory::read(const char* name, char* dst, size_t maxLen)
{
    gMem.begin( MEMORY_SECTION, true );
    size_t sz = gMem.getBytes( name, dst, maxLen );
    gMem.end();
    return sz;
}

size_t Memory::write(const char* name, const char* val, size_t len)
{
    // ijh debug start
    char dbgbuf[100] = {0};
    memcpy(dbgbuf, val, len);
    DBG(String("Flashing") + name + ":" + dbgbuf );
    // ijh debug end

    gMem.begin( MEMORY_SECTION, false );
    size_t sz = gMem.putBytes( name, val, len );
    gMem.end();
    return sz;
}

bool Memory::readKey(const char *name, uint8_t *dst)
{
    return LORA_KEY_LEN == read(name, (char*)dst, LORA_KEY_LEN);
}

bool Memory::writeKey(const char *name, const uint8_t *src)
{
    return LORA_KEY_LEN == write( name, (const char*) src, LORA_KEY_LEN);
}

bool Memory::readAddrHex(char *toAddr)
{
    return read("addr", (char*)toAddr, LORA_ADDR_HEX_LEN);
}


bool Memory::writeAddrHex( String fromAddr)
{
    return write( "addr", fromAddr.c_str(), LORA_ADDR_HEX_LEN);
}

bool Memory::writeName(String from)
{
    size_t len = from.length();

    if( len >= LORA_NAME_MAX_LEN ) {
        return false;
    }

    if( write("nameLen", (const char*) &len, sizeof(len)) != sizeof(size_t) ); {
        return false;
    }
    return write("name", from.c_str(), len) == len;
}

bool Memory::readName(String& to)
{
    int len = 0;
    char buff[LORA_NAME_MAX_LEN + 1] = {0};
    if( sizeof(len) != read("nameLen", (char*) &len, sizeof(len)) ) {
        return false;
    }

    if( len >= LORA_NAME_MAX_LEN ) {
        return false;
    }

    if( len != read("name", buff, len) ) {
        return false;
    }

    to = buff;

    return true;
}

bool Memory::writeSex(bool isMale)
{
    char x = isMale ? 'm' : 'f';
    return write("sex", &x, 1 );
}

bool Memory::readSex(bool &isMale)
{
    char x = 0;
    if( !read("sex", &x, 1 ) ) {
        return false;
    }

    isMale = x == 'm' ? true : false;

    return true;
}



