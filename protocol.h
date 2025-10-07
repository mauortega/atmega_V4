#ifndef PROTOCOL_H
#define PROTOCOL_H

/*
 * Comandos vindos pela softwareserial (rpi/main)
 */
#define CMD_SERIAL_CAN_125K 'P'
#define CMD_SERIAL_CAN_250K 'Q'
#define CMD_SERIAL_CAN_500K 'R'
#define CMD_SERIAL_CAN_1000K 'S'

#define CMD_SERIAL_WDT_RESET 'U'
#define CMD_SERIAL_WDT_ENABLE 'V'
#define CMD_SERIAL_WDT_DISABLE 'W'

#define CMD_SERIAL_CANDEBUG_ENABLE 'X'
#define CMD_SERIAL_CANDEBUG_DISABLE 'Y'

#define CMD_SERIAL_VAL_START_CATRACA 'Z'
#define CMD_SERIAL_TD2UCP 'A'
#define CMD_SERIAL_UCP2TD 'B'

#define SERIAL_DATA_SOFTPOWEROFF_ENABLE 'C'
#define SERIAL_DATA_SOFTPOWEROFF_DISABLE 'D'

#define CMD_SERIAL_VAL_RFID_ENABLE 'E'
#define CMD_SERIAL_VAL_RFID_DISABLE 'e'

#define CMD_SERIAL_VAL_RFID_DIRECT_ENABLE 'F'
#define CMD_SERIAL_VAL_RFID_DIRECT_DISABLE 'f'

#define CMD_SERIAL_CAN_NORMAL 'G'
#define CMD_SERIAL_CAN_LISTEN 'g'

#define CMD_SERIAL_PROTOCOL_LEGACY 'H'
#define CMD_SERIAL_PROTOCOL_NEW 'h'

#define CMD_SERIAL_PROTOCOL_RESET 'I'

///////
#define PROTOCOL_START 0x01
#define PROTOCOL_STOP 0x04
#define PROTOCOL_ESCAPE 0x10

typedef struct
{
    uint8_t id;
    int16_t valor;
    float medida;
} DataPacket;

enum PacketType
{
    PACKET_TYPE_DATA = 0x01,
    PACKET_TYPE_ACK = 0x02,
    PACKET_TYPE_NACK = 0x03
};

enum FrameType
{
    FrameTypeCAN,
    FrameTypeIO,
    FrameTypeValidator,
    FrameTypeRFID
};

//////
void ProtocolSerialInit(Stream &debug_port);
void ProtocolLoop();

int ProtocolDecodeBuffer(void *src, void *dest, int len);
int ProtocolEncodeBuffer(void *src, void *dest, int len);

byte ProtocolCheckCRC(unsigned char *buffer, int len);
unsigned char ProtocolCalcCRC(void *ptr, unsigned int len);

void SerialProtocolPrintHex(Stream *serial, unsigned char *data, unsigned int length, byte frameType);
void SerialPrintInt16(Stream *serial, unsigned int value);
void SerialPrintByte(Stream *serial, unsigned char value);

// void ProtocoloPrintAscii(Stream *serial, unsigned char *data, unsigned int length);
void ProtocoloPrintHex(Stream *serial, unsigned char *data, unsigned int length);
void ProtocoloSendDebug(Stream *serial, char *data);
unsigned char ProtocolCalcCRC(void *ptr, unsigned int len);

void ProtocolSendVersion();
void ProtocoloSendRFIDMsg(unsigned char *cardId, unsigned int rfidDataLen);
void ProtocoloSendValidatorValue(unsigned char *packet, unsigned int packetLen);
void ProtocoloSendValidatorToRFID(void *bfr, unsigned int packetLen);

void ProtocoloSendRFIDToValidator(void *bfr, unsigned int packetLen);

void ProtocoloProcessUCP2TD(unsigned char *bfr, unsigned char packetLen);
void ProtocoloProcessTD2UCP(unsigned char *bfr, unsigned char packetLen);

void forceReset();

// void ProtocolTest(Stream* serial);

#endif
