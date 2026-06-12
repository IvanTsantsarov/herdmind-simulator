#ifndef LORADEV_DEF_H
#define LORADEV_DEF_H

#include <QtTypes>

namespace {
constexpr quint8 CID_LinkADRReq    = 0x03;
constexpr quint8 CID_LinkADRAns    = 0x03;
constexpr quint8 CID_RXParamSetupReq = 0x05;
constexpr quint8 CID_DevStatusReq = 0x06;
constexpr quint8 CID_NewChannelReq = 0x07;
constexpr quint8 CID_NewChannelAns = 0x07;
constexpr quint8 CID_RXTimingSetupReq = 0x08;
constexpr quint8 CID_PingSlotChannelReq = 0x11;
// Uplink FCtrl bits
constexpr quint8 FCtrl_ACK_bit       = (1 << 5);
constexpr quint8 FCtrl_FOptsLen_mask = 0x0F;

// Keep FOpts <= 15 bytes
constexpr int MaxFOptsLen = 15;
}


#endif // LORADEV_DEF_H
