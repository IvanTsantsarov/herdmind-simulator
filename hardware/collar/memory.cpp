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


