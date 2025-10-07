#ifndef validator_protocol_H
#define validator_protocol_H

#include <Arduino.h>

//-------------------------------------------------------------------------------
// /* Protocol Constants */

#define PRODATA_BUFFER_LENGTH 100  // acho que pode ser menor, transmitimos setores de 48 bytes + overhead de 5 bytes

// Protocolo padrao
#define PRODATA_STD_STX_ID 0x01
#define PRODATA_STD_ETX_ID 0x04
#define PRODATA_STD_ESCAPE_ID 0x10

// protocolo catraca/validador
#define PRODATA_STX_ID 0x02
#define PRODATA_ETX_ID 0x03

// protocolo ucp/validador
#define PRODATA_UCP_STX_ID 0xAA
#define PRODATA_UCP_ETX_ID 0x55

// protocolo transdata
#define TRANSDATA_STX_ID 'R'

// pino RS485
#define PRODATA_RS485_RXTX A3

#define NONE_TYPE (0)

//-------------------------------------------------------------------------------
// Protocolo validador v2
#define PRODATA_V2_VERSION 0x20

#define PRODATA_V2_TYPE_LEGACY 0x30
#define PRODATA_V2_TYPE_POOLING 0x40
#define PRODATA_V2_TYPE_OPERATION 0x41
#define PRODATA_V2_TYPE_PASSANGER 0x42

#define PRODATA_V2_ACK_POOLING 0xC0
#define PRODATA_V2_ACK_OPERATION 0xC1
#define PRODATA_V2_ACK_PASSANGER 0xC2

#define PRODATA_V2_STX(array) array[0]
#define PRODATA_V2_CMD(array) array[1]
// #define PRODATA_V2_VERSION(array) array[2]
#define PRODATA_V2_SEQUENCE(array) array[3]
#define PRODATA_V2_LENGTH(array)  (*((uint16_t*)&(array)[4]))
#define PRODATA_V2_DATA(array, index) array[6 + index]
#define PRODATA_V2_CRC(array) array[6 + PRODATA_V2_LENGTH(array) + 1]
#define PRODATA_V2_ETX(array) array[3 + PRODATA_LENGTH(array)]
#define PRODATA_V2_PACKET_LENGTH(array) (3 + PRODATA_LENGTH(array) + 2)

//-------------------------------------------------------------------------------
// Protocolo validador rfid
#define PRODATA_STX(array) array[0]
#define PRODATA_CMD(array) array[1]
#define PRODATA_LENGTH(array) array[2]
#define PRODATA_DATA(array, index) array[3 + index]
#define PRODATA_ETX(array) array[3 + PRODATA_LENGTH(array)]
#define PRODATA_CRC(array) array[3 + PRODATA_LENGTH(array) + 1]
#define PRODATA_PACKET_LENGTH(array) (3 + PRODATA_LENGTH(array) + 2)

//-------------------------------------------------------------------------------
// protocolo validador std (td)
#define PRODATA_UCP_STX(array) array[0]
#define PRODATA_UCP_CMD(array) array[1]
#define PRODATA_UCP_SEQNUM(array) array[2]
// #define UCP_LEN(array) array[2]*8 + array[3]
#define PRODATA_UCP_LENGTH(array) array[3]
#define PRODATA_UCP_DATA(array, index) array[4 + index]
#define PRODATA_UCP_ETX(array) array[4 + PRODATA_UCP_LENGTH(array)]
#define PRODATA_UCP_CRC(array) array[4 + PRODATA_UCP_LENGTH(array) + 1]
#define PRODATA_UCP_PACKET_LENGTH(array) (4 + PRODATA_UCP_LENGTH(array) + 2)

//-------------------------------------------------------------------------------
typedef enum {
    PDT_MIFARE_REQUEST = 0x31,
    PDT_MIFARE_ANTICOLLISION = 0x32,
    PDT_MIFARE_SELECT = 0x33,
    PDT_MIFARE_LOAD_KEY = 0x34,
    PDT_MIFARE_AUTHENTICATION = 0x35,
    PDT_MIFARE_READ = 0x36,
    PDT_MIFARE_WRITE = 0x37,
    PDT_MIFARE_HALT = 0x38,
    PDT_MIFARE_DECREMENT = 0x39,
    PDT_MIFARE_INCREMENT = 0x3A,
    PDT_MIFARE_TRANSFER = 0x3B,
    PDT_MIFARE_RESTORE = 0x3C,
    PDT_MIFARE_READ_SECTOR = 0x3D,
    PDT_MIFARE_WRITE_SECTOR = 0x3E,
    PDT_MIFARE_RESET_RF = 0x3F,
    PDT_MIFARE_POWER_DOWN = 0x40,
    PDT_DISPLAY_CLEAR = 0x63,
    PDT_DISPLAY_WRITE_LINE = 0x6C,
    PDT_CMD_BUZZER = 0x74,
    PDT_CMD_BUZZER_TMP = 0x75,
    PDT_CMD_REBOOT = 0x96,
    PDT_CMD_GET_SERIAL = 0x98,
    PDT_CMD_GET_VERSION = 0x99,
    PDT_CMD_KEEP_ALIVE = 0xFF
} MifareCommand;

typedef enum {
    PDT_STATUS_MIFARE_OK = 0x00,
    PDT_STATUS_MIFARE_ACCESS_ERROR = 0xEA,
    PDT_STATUS_MIFARE_VALUE_ERROR = 0x84,
    PDT_STATUS_MIFARE_INVALID_CMD = 0xFF,
    PDT_STATUS_MIFARE_ANTI_COLISION = 0xFF,
    PDT_STATUS_MIFARE_SELECT_ERROR = 0xFF,
    PDT_STATUS_MIFARE_AUTHENTICATION_ERROR = 0xFC,
    PDT_STATUS_MIFARE_TIMER_OFF = 0xFF,
    PDT_STATUS_MIFARE_FLAGS_ERROR = 0xFF,
    PDT_STATUS_MIFARE_FIFO_ERROR = 0xFF,
    PDT_STATUS_MIFARE_RF_NO_CARD = 0xFF,
    PDT_STATUS_MIFARE_KEY_ERROR = 0xF7
} MifareError;

//-------------------------------------------------------------------------------

void ValidatorProtocolSetup(Stream &protocol_port, Stream &debug_port);
void ValidatorProtocolProcess();

// unsigned char ValidatorProtocolGetType(void *src);

//void ValidatorProtocolPrintDescription(void *src, Stream &serial_port);
// void ValidatorProtocolPrintPacket(void *src, Stream &serial_port);

// put entire packet here
// call this function to validade the received packet
//bool ValidatorProtocolPacketCheck(void *src, int len);

void ValidatorProtocolDumpHexPacket(void *src);

void ValidatorProtocolSendPacket(void *src, int len);

// returns the length of the destination array
int ValidatorProtocolBuild(void *src, void *dest, unsigned char data_len, unsigned char cmd);

int ValidatorProtocolV2Build(void *src, void *dest, unsigned int data_len, unsigned char cmd, unsigned char version, unsigned char sequence);

// returns the length of the destination array
// int ValidatorProtocolUcpBuild(void *src, void *dest, unsigned int data_len, unsigned char cmd);

//void ProdataSimulator();

void ValidatorProtocolCardDetected();
void ProdataUCPSendAck();

#endif  // prodata_protocol_H
