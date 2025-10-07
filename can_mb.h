#ifndef CANMB_H
#define CANMB_H

#include "can.h"

#define CAN_MB_PACKET_GEAR 0x22C
#define CAN_MB_PACKET_HANDBRAKE 0x250
#define CAN_MB_PACKET_FOOTBRAKE 0x250
#define CAN_MB_PACKET_VEHICLE_SPEED 0x250
#define CAN_MB_PACKET_ENGINE_SPEED 0x250
#define CAN_MB_PACKET_RETARDER 0x304
#define CAN_MB_PACKET_PEDAL_POSITION 0x450
#define CAN_MB_PACKET_AMBIENT_TEMPERATURE 0x550
#define CAN_MB_PACKET_ENGINE_TEMPERATURE 0x554
#define CAN_MB_PACKET_OIL_PRESSURE 0x554
#define CAN_MB_PACKET_AIR_SUPPLY_PRESSURE 0x5A0
#define CAN_MB_PACKET_ALTERNATOR 0x5A0
#define CAN_MB_PACKET_FUEL_LEVEL 0x6A0
#define CAN_MB_PACKET_HIGHRES_DISTANCE 0x6B5
#define CAN_MB_PACKET_FUEL_CONSUMPTION 0x65E
#define CAN_MB_PACKET_ADBLUECONSAVG 0x65E
#define CAN_MB_PACKET_ADBLUE_LEVEL 0x65F

void CanMBDataParse(unsigned long canId, unsigned char *data, Stream *serial, struct CANValue &pointer);

unsigned long CanMBTotalPackets();
unsigned long CanMBTotalTime();

bool CanMBPacketReceived();
void CanMBResetPacketReceived();

#endif
