#include <Arduino.h>
#include "validator.h"
#include "validatorProtocol.h"
#include "validatorRfid.h"
#include "protocol.h"

#define VAL_BAUD 19200
#define VAL_PORT Serial
#define VAL_BUFF_LEN 380

static Stream *serialValidator;
static Stream *serialRpi;

unsigned char validatorBuffer[VAL_BUFF_LEN];
unsigned int validatorPointer;

static bool validatorStdPacketReceived = false;
static bool validatorUcpPacketReceived = false;
static bool validatorRfidPacketReceived = false;
static bool transdataPacketReceived = false;

static unsigned char validatorProtocolType = 0;

bool validatorInitialized = false;
bool catracaProcessStarted = false;

// const unsigned char validatorTestPacket[] = { 0x01, 0x30, 0x10, 0x21, 0xf9, 0x10, 0x21, 0x10, 0x21, 0x00, 0x0a, 0x71, 0x59, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x21, 0x62, 0xbb, 0x00, 0x10, 0x21, 0x3d, 0x75, 0xc1, 0x04 };
const unsigned char validatorTestPacket[] = {0x01, 0x30, 0x00, 0x46, 0x10, 0x21, 0x00, 0x37, 0x89, 0x9F, 0xCB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x21, 0x62, 0x4E, 0x00, 0x00, 0xF7, 0x10, 0x30, 0xE9, 0x04};

void ValidatorSetup(Stream &debugSerial)
{
  serialRpi = &debugSerial;
  serialValidator = &VAL_PORT;

  VAL_PORT.begin(VAL_BAUD);

  ValidatorRfidSetup(*serialRpi);
  ValidatorProtocolSetup(*serialValidator, *serialRpi);

  pinMode(VALIDATOR_RS485_RXTX, OUTPUT);

  ValidatorSetReceptionMode();

  validatorInitialized = true;
}

void ValidatorSetTransmitionMode()
{
  digitalWrite(VALIDATOR_RS485_RXTX, HIGH);
}

void ValidatorSetReceptionMode()
{
  digitalWrite(VALIDATOR_RS485_RXTX, LOW);
}

byte ValidatorCRC(void *data, int len)
{
  byte *buffer = (byte *)data;
  byte value = 0;
  byte crc = 0;

  /* Sum all byte values */
  for (int i = 0; i < len; i++)
  {
    value = buffer[i];
    crc += value;
  }
  return crc;
}

bool ValidatorStdPacketReceived()
{
  return validatorStdPacketReceived;
}

bool ValidatorRfidPacketReceived()
{
  return validatorRfidPacketReceived;
}

bool ValidatorUcpPacketReceived()
{
  return validatorUcpPacketReceived;
}

bool ValidatorTransdataPacketReceived()
{
  return transdataPacketReceived;
}

void ValidatorTransdataPacketReceivedReset()
{
  transdataPacketReceived = false;
}

void ValidatorStdPacketReceivedReset()
{
  validatorStdPacketReceived = false;
}

void ValidatorRfidPacketReceivedReset()
{
  validatorRfidPacketReceived = false;
}

void ValidatorUcpPacketReceivedReset()
{
  validatorUcpPacketReceived = false;
}

unsigned char *ValidatorGetPacket()
{
  validatorStdPacketReceived = false;

  return &validatorBuffer[0];
}

unsigned int ValidatorPacketLen()
{
  return validatorPointer;
}

void ValidatorSimulator()
{
  ValidatorSetTransmitionMode();

  for (unsigned char c = 0; c < sizeof(validatorTestPacket); c++)
    serialValidator->write(validatorTestPacket[c]);

  ValidatorSetReceptionMode();
}

void ValidatorTestPacket(Stream *serial)
{
  serial->println(F("Validator - TEST - Decoding packet"));

  for (int c = 0; c < sizeof(validatorTestPacket); c++)
    validatorBuffer[c] = validatorTestPacket[c];

  validatorPointer = sizeof(validatorTestPacket);

  bool packetDecoded = ProtocolCheckCRC((unsigned char *)&validatorTestPacket, sizeof(validatorTestPacket));

  if (packetDecoded)
  {
    serial->println(F("Validator - TEST - Packet decoded sucessfully"));

    validatorStdPacketReceived = true;
  }
  else
  {
    serial->println(F("Validator - TEST - Error decoding packet"));
  }
}

// byte ValidatorCheckCRC(unsigned char* buffer, int len) {
//   if (len > 3) {
//     if (buffer[len - 3] != 0x10) {
//       byte crc = buffer[len - 2];

//       if (crc == ValidatorCRC(&buffer[1], len - 3))
//         return true;
//     } else {
//       byte crc = buffer[len - 2] - 0x20;

//       if (crc != ValidatorCRC(&buffer[1], len - 4))
//         return true;
//     }
//   }

//   return false;
// }

/////////////////////////////////////////////////////////////////
void ValidatorLoop()
{
  static unsigned char len = 2;
  static unsigned char len2 = 0;
  static unsigned long timeout = 0;

  if (validatorInitialized == false)
  {
    return;
  }

  if (validatorProtocolType > 0 && timeout > 0 && (millis() - timeout) > 2000)
  {
    validatorProtocolType = 0;
    validatorPointer = 0;
    timeout = 0;
  }

  while (serialValidator->available() 
  && validatorStdPacketReceived == false 
  && validatorRfidPacketReceived == false 
  && validatorUcpPacketReceived == false 
  && transdataPacketReceived == false)
  {
    unsigned char data = serialValidator->read();

    switch (validatorProtocolType)
    {
    case 0:
      if (data == PRODATA_STD_STX_ID || data == PRODATA_STX_ID || data == PRODATA_UCP_STX_ID || data == TRANSDATA_STX_ID)
      {
        validatorPointer = 0;
        validatorRfidPacketReceived = false;
        validatorUcpPacketReceived = false;
        validatorStdPacketReceived = false;
        transdataPacketReceived = false;
        validatorProtocolType = data;
        validatorBuffer[validatorPointer++] = data; // grava o header no buffer tambem

        timeout = millis();
      }
      break;

      ///////// STD - 01 30 10 21 F9 10 21 10 21 00 0A 71 59 00 00 00 00 00 10 21 62 BB 00 10 21 3D 75 C1 04
    case PRODATA_STD_STX_ID:
      if (validatorPointer < VAL_BUFF_LEN)
      {
        validatorBuffer[validatorPointer++] = data;
      }
      else
      {
        validatorProtocolType = 0;
      }

      if (data == PRODATA_STD_ETX_ID && (validatorBuffer[1] == 0x30 || validatorBuffer[1] == 0x40 || validatorBuffer[1] == 0x41 || validatorBuffer[1] == 0x42))
      {
#ifndef __WITHOUT_CRC__
        if (validatorPointer > 3)
        {
          if (validatorBuffer[validatorPointer - 3] != PRODATA_STD_ESCAPE_ID)
          {
            byte crc = validatorBuffer[validatorPointer - 2];
            if (crc == ValidatorCRC(&validatorBuffer[1], validatorPointer - 3))
              validatorStdPacketReceived = true;
          }
          else
          {
            byte crc = validatorBuffer[validatorPointer - 2] - 0x20;
            if (crc == ValidatorCRC(&validatorBuffer[1], validatorPointer - 4))
              validatorStdPacketReceived = true;
          }
        }
#else
          // sem CRC pra ser mais rapido
          validatorStdPacketReceived = true;
#endif

        validatorProtocolType = 0;
      }
      break;

      /////////  RFID - 02 31 01 52 03 61
    case PRODATA_STX_ID:
      if (validatorPointer < VAL_BUFF_LEN)
      {
        validatorBuffer[validatorPointer] = data;
      }
      else
      {
        validatorProtocolType = 0;
      }

      if (validatorPointer == 2)
      {
        len = data;
      }

      validatorPointer++;

      if (validatorPointer >= (5 + len) && validatorBuffer[validatorPointer - 2] == PRODATA_ETX_ID)
      {
        validatorRfidPacketReceived = true;
        validatorProtocolType = 0;
        // usar função de check packet? CRC?
      }

      break;

    /////////
    // AA CA 00 00 55 35 - pedido de ack
    case PRODATA_UCP_STX_ID:
      if (validatorPointer < VAL_BUFF_LEN)
      {
        validatorBuffer[validatorPointer] = data;
      }
      else
      {
        validatorProtocolType = 0;
      }

      if (validatorPointer == 3)
      {
        len = data;
      }

      validatorPointer++;

      if (validatorPointer >= (6 + len) &&  validatorBuffer[validatorPointer - 2] == PRODATA_UCP_ETX_ID)
      {
          validatorUcpPacketReceived = true;
          validatorProtocolType = 0;
          // usar função de check packet?
      }
      break;
    case TRANSDATA_STX_ID:
      // serialRpi->println(data, HEX);
      
      if (validatorPointer < VAL_BUFF_LEN)
      {
        validatorBuffer[validatorPointer] = data;
      }
      else
      {
        validatorProtocolType = 0;
      }

      if (validatorPointer == 8)
      {
        len = data;
      }
      else if (validatorPointer == 9)
      {
        len2 = data;
      }

      validatorPointer++;

      if (validatorPointer >= (14 + (len | (len2 << 8))))
      {
          transdataPacketReceived = true;
          validatorProtocolType = 0;
      }
      break;
    }
  }

  ValidatorProtocolProcess();
}
