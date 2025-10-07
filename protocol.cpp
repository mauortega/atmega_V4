#include <Arduino.h>
#include <avr/wdt.h>

#include "validatorProtocol.h"
#include "validatorRfid.h"
#include "protocol.h"

#include "app.h"
#include "can.h"
#include "rpiWdt.h"
#include "io.h"
#include "config.h"

#include "Frame.h"

// const uint8_t ProtocolMaxRetries = 3;
uint8_t ProtocolSequenceNumber = 0;
// uint8_t ProtocolLastSequenceNumber = 255;

static Stream *serialRpi;

/*****************************************************************************/
void ProtocolSerialInit(Stream &debug_port)
{
  serialRpi = &debug_port;
}

/*****************************************************************************/
void ProtocolLoop()
{
  static unsigned char currCmd = 0;
  static bool receiving = false;
  static unsigned char bufSerial[50];
  static unsigned char bufSerialLen = 0;
  static unsigned char configCmd = 0;

  if (serialRpi->available())
  {
    unsigned char dataR = serialRpi->read();

    if (receiving == true)
    {
      if (dataR == '\n' || dataR == '\r')
      {
        dataR = currCmd;
        bufSerial[bufSerialLen] = 0;
      }
      else if (bufSerialLen >= sizeof(bufSerial) - 1) 
      {
        receiving = false;
        bufSerialLen = 0;
        return;
      }
      else
      {
        bufSerial[bufSerialLen++] = dataR;
        return;
      }
    }

    switch (dataR)
    {
    case CMD_SERIAL_WDT_RESET:
      RpiWdtPulse();
      break;

    case CMD_SERIAL_WDT_ENABLE:
      serialRpi->println(F("[CMD] WDT_ENABLED"));
      RpiWtdSetEnabled(true);
      break;

    case CMD_SERIAL_WDT_DISABLE:
      serialRpi->println(F("[CMD] WDT_DISABLED"));
      RpiWtdSetEnabled(false);
      break;

    // case CMD_SERIAL_VAL_RFID_ENABLE:
    //   if (configCmd == dataR)
    //   {
    //     serialRpi->println(F("[CMD] VAL_RFID_ENABLED"));
    //     configData.validatorRfidEnabled = 1;
    //     ConfigSave();
    //     configCmd = 0;
    //   }
    //   else
    //   {
    //     configCmd = dataR;
    //   }
    //   break;

    // case CMD_SERIAL_VAL_RFID_DISABLE:
    //   if (configCmd == dataR)
    //   {
    //     serialRpi->println(F("[CMD] VAL_RFID_DISABLED"));
    //     configData.validatorRfidEnabled = 0;
    //     ConfigSave();

    //     configCmd = 0;
    //   }
    //   else
    //   {
    //     configCmd = dataR;
    //   }
    //   break;

    // case CMD_SERIAL_VAL_RFID_DIRECT_ENABLE:
    //   if (configCmd == dataR)
    //   {
    //     serialRpi->println(F("[CMD] VAL_RFID_DIRECT_ENABLED"));
    //     configData.validatorRfidDirect = 1;
    //     ConfigSave();

    //     configCmd = 0;
    //   }
    //   else
    //   {
    //     configCmd = dataR;
    //   }
    //   break;

    // case CMD_SERIAL_VAL_RFID_DIRECT_DISABLE:
    //   if (configCmd == dataR)
    //   {
    //     serialRpi->println(F("[CMD] VAL_RFID_DIRECT_DISABLED"));
    //     configData.validatorRfidDirect = 0;
    //     ConfigSave();

    //     configCmd = 0;
    //   }
    //   else
    //   {
    //     configCmd = dataR;
    //   }
    //   break;

    // case CMD_SERIAL_CAN_NORMAL:
    //   if (configCmd == dataR)
    //   {
    //     serialRpi->println(F("[CMD] CAN_NORMAL"));
    //     configData.canListenOnlyMode = 0;
    //     ConfigSave();
    //     CanSetListenMode(configData.canListenOnlyMode);
    //     CanChangeNormalMode();

    //     configCmd = 0;
    //   }
    //   else
    //   {
    //     configCmd = dataR;
    //   }
    //   break;

    // case CMD_SERIAL_CAN_LISTEN:
    //   if (configCmd == dataR)
    //   {
    //     serialRpi->println(F("[CMD] CAN_LISTEN"));
    //     configData.canListenOnlyMode = 1;
    //     ConfigSave();
    //     CanSetListenMode(configData.canListenOnlyMode);
    //     CanChangeListenMode();

    //     configCmd = 0;
    //   }
    //   else
    //   {
    //     configCmd = dataR;
    //   }
    //   break;

    // case CMD_SERIAL_PROTOCOL_LEGACY:
    //   if (configCmd == dataR)
    //   {
    //     serialRpi->println(F("[CMD] PROTOCOL_LEGACY"));
    //     configData.serialProtocolLegacy = 1;
    //     ConfigSave();

    //     configCmd = 0;
    //   }
    //   else
    //   {
    //     configCmd = dataR;
    //   }
    //   break;

    // case CMD_SERIAL_PROTOCOL_NEW:
    //   if (configCmd == dataR)
    //   {
    //     serialRpi->println(F("[CMD] PROTOCOL_NEW"));
    //     configData.serialProtocolLegacy = 0;
    //     ConfigSave();

    //     configCmd = 0;
    //   }
    //   else
    //   {
    //     configCmd = dataR;
    //   }
    //   break;

    case CMD_SERIAL_PROTOCOL_RESET:
      if (configCmd == dataR)
      {
        serialRpi->println(F("[CMD] PROTOCOL_RESET"));
        forceReset();

        configCmd = 0;
      }
      else
      {
        configCmd = dataR;
      }
      break;

    case CMD_SERIAL_VAL_START_CATRACA:
      serialRpi->println(F("[CMD] VALID RFID INIT"));
      ValidatorProtocolCardDetected();
      break;

    case CMD_SERIAL_UCP2TD:
      if (receiving == false)
      {
        receiving = true;
        bufSerialLen = 0;
        currCmd = CMD_SERIAL_UCP2TD;
      }
      else
      {
        receiving = false;
        ProtocoloProcessUCP2TD(bufSerial, bufSerialLen);
      }
      break;

    case CMD_SERIAL_TD2UCP:
      if (receiving == false)
      {
        receiving = true;
        bufSerialLen = 0;
        currCmd = CMD_SERIAL_TD2UCP;
      }
      else
      {
        receiving = false;
        ProtocoloProcessTD2UCP(bufSerial, bufSerialLen);
      }
      break;

#ifndef OPTION_DISABLE_CAN
    case CMD_SERIAL_CAN_125K:
    case CMD_SERIAL_CAN_250K:
    case CMD_SERIAL_CAN_500K:
    case CMD_SERIAL_CAN_1000K:
      CanChangeSpeed(dataR, serialRpi);
      break;

#ifndef OPTION_CAN_SIMULATOR
    case CMD_SERIAL_CANDEBUG_ENABLE:
      SetCanDebug(true);
      break;
    case CMD_SERIAL_CANDEBUG_DISABLE:
      SetCanDebug(false);
      break;
#endif
#endif
    default:
      receiving = false;
      configCmd = 0;
      break;
    }
  }
}

/*****************************************************************************/
int ProtocolDecodeBuffer(void *src, void *dest, int len)
{
  /* Destination buffer data length */
  int destBufferLen = 0;
  unsigned char *inbuffer = (unsigned char *)src;
  unsigned char *oubuffer = (unsigned char *)dest;
  //  unsigned char escapeCode = false;

  /* Search encoded bytes */
  for (int i = 0; i < len; i++)
  {
    if (inbuffer[i] != PROTOCOL_START && inbuffer[i] != PROTOCOL_STOP)
    {
      /* If 0x10 encode prefix found */
      if (inbuffer[i] == PROTOCOL_ESCAPE)
        /* Sub 0x20 from byte value to decode */
        oubuffer[destBufferLen++] = inbuffer[++i] - 0x20;
      else
        /* Else only add byte to buffer */
        oubuffer[destBufferLen++] = inbuffer[i];
    }
  }

  /* Return decoded data length */
  return destBufferLen;
}

/*****************************************************************************/
int ProtocolEncodeBuffer(void *src, void *dest, int len)
{
  /* Destination buffer data length */
  int destBufferLen = 0;

  unsigned char *inbuffer = (unsigned char *)src;
  unsigned char *oubuffer = (unsigned char *)dest;

  oubuffer[destBufferLen++] = PROTOCOL_START;

  /* Search bytes to encode */
  for (int i = 0; i < len; i++)
  {
    switch (inbuffer[i])
    {
    /* If is an 'encodable' byte */
    case 0x01:
    case 0x04:
    case 0x10:
      /* Add 0x10 to prefix encoded byte */
      oubuffer[destBufferLen++] = PROTOCOL_ESCAPE;
      /* Sum byte value with 0x20 to encode it */
      oubuffer[destBufferLen++] = inbuffer[i] + 0x20;
      break;
    /* Else only add byte to buffer */
    default:
      oubuffer[destBufferLen++] = inbuffer[i];
      break;
    }
  }

  oubuffer[destBufferLen++] = PROTOCOL_STOP;

  /* Return encoded data length */
  return destBufferLen;
}

void ProtocoloPrintHex(Stream *serial, unsigned char *data, unsigned int length)
{
  byte highNibble;
  byte lowNibble;
  char c;

  for (unsigned int i = 0; i < length; i++)
  {
    highNibble = (data[i] >> 4) & 0x0F;
    lowNibble = data[i] & 0x0F;

    if (highNibble < 10)
      c = '0' + highNibble;
    else
      c = 'A' + (highNibble - 10);

    serial->print(c);

    if (lowNibble < 10)
      c = '0' + lowNibble;
    else
      c = 'A' + (lowNibble - 10);

    serial->print(c);
  }

  serial->println();

  serial->flush();
  delay(3);
}

byte ProtocolCRC(void *data, int len)
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

byte ProtocolCheckCRC(unsigned char *buffer, int len)
{
  if (len > 3)
  {
    if (buffer[len - 3] != 0x10)
    {
      byte crc = buffer[len - 2];

      if (crc == ProtocolCRC(&buffer[1], len - 3))
        return true;
    }
    else
    {
      byte crc = buffer[len - 2] - 0x20;

      if (crc != ProtocolCRC(&buffer[1], len - 4))
        return true;
    }
  }

  return false;
}
/*****************************************************************************/
// void ProtocoloPrintHex(Stream *serial, unsigned char *data, unsigned int length)
// {
//   char tmp[length * 2 + 1];

//   byte first;
//   byte second;

//   for (unsigned int i = 0; i < length; i++)
//   {
//     first = (data[i] >> 4) & 0x0f;

//     second = data[i] & 0x0f;

//     // base for converting single digit numbers to ASCII is 48
//     // base for 10-16 to become lower-case characters a-f is 87
//     // note: difference is 39

//     tmp[i * 2] = first + 48;
//     tmp[i * 2 + 1] = second + 48;

//     if (first > 9)
//       tmp[i * 2] += 07;

//     if (second > 9)
//       tmp[i * 2 + 1] += 07;
//   }

//   tmp[length * 2] = 0;
//   serial->println(tmp);
//   delay(3);
//   serial->flush();
// }

/*****************************************************************************/
void ProtocoloPrintBin(Stream *serial, unsigned char *data, unsigned int length)
{
  for (unsigned int i = 0; i < length; i++)
  {
    serial->print(data[i]);
  }

  serial->print("\r\n");
  // delay(3);
  serial->flush();
}

/*****************************************************************************/
int hexStringToBinary(const unsigned char *hexString, unsigned char bufLen, unsigned char *output)
{
  if (bufLen % 2 != 0)
  {
    return -1; // Return error if the input length is not even
  }

  unsigned char outIndex = 0;

  for (unsigned char i = 0; i < bufLen; i += 2)
  {
    unsigned char highNibble, lowNibble;

    // Convert high nibble
    if (hexString[i] >= '0' && hexString[i] <= '9')
    {
      highNibble = hexString[i] - '0';
    }
    else if (hexString[i] >= 'A' && hexString[i] <= 'F')
    {
      highNibble = hexString[i] - 'A' + 10;
    }
    else if (hexString[i] >= 'a' && hexString[i] <= 'f')
    {
      highNibble = hexString[i] - 'a' + 10;
    }
    else
    {
      return -1; // Invalid character in hex string
    }

    // Convert low nibble
    if (hexString[i + 1] >= '0' && hexString[i + 1] <= '9')
    {
      lowNibble = hexString[i + 1] - '0';
    }
    else if (hexString[i + 1] >= 'A' && hexString[i + 1] <= 'F')
    {
      lowNibble = hexString[i + 1] - 'A' + 10;
    }
    else if (hexString[i + 1] >= 'a' && hexString[i + 1] <= 'f')
    {
      lowNibble = hexString[i + 1] - 'a' + 10;
    }
    else
    {
      return -1; // Invalid character in hex string
    }

    // Combine the nibbles into a single byte and store in output array
    output[outIndex++] = (highNibble << 4) | lowNibble;
  }

  return outIndex; // Return the length of the output array
}

/*****************************************************************************/
void ProtocoloSendDebug(Stream *serial, char *data)
{
  char str[80];
  memset(str, 0, sizeof(str));
  strncpy(str, data, 79);
  serial->print(F("[DEBUG] "));
  serial->println(str);
  serial->flush();
  // serial.print("\r\n");
  // delay(3);
}

/*****************************************************************************/
unsigned char ProtocolCalcCRC(void *ptr, unsigned int len)
{
  unsigned char crc = 0x00;

  for (unsigned int c = 0; c < len - 1; c++)
    crc ^= *(unsigned char *)ptr + c;

  return crc;
}

/////////////////////////////////////////////////////////////////
void ProtocolSendVersion()
{
  serialRpi->print(F("[ATMEGA_VERSION]:"));
  serialRpi->println(NX7000_ATMEGA_VERSION);

  if (CanGetCanStarted())
  {
    switch (CanGetCanBitRate())
    {
    case CAN_NOT_DETECTED:
      serialRpi->println(F("[TELEMETRY]:NOT_DETECTED"));
      break;
    case CAN_125K:
      serialRpi->println(F("[TELEMETRY]:AUTO125K"));
      break;
    case CAN_250K:
      serialRpi->println(F("[TELEMETRY]:AUTO250K"));
      break;
    case CAN_500K:
      serialRpi->println(F("[TELEMETRY]:AUTO500K"));
      break;
    case CAN_1000K:
      serialRpi->println(F("[TELEMETRY]:AUTO1000K"));
      break;
    default:
      serialRpi->println(F("[TELEMETRY]:UNDEFINED"));
      break;
    }
  }
  else
  {
    serialRpi->println(F("[TELEMETRY]:ERROR"));
  }

  serialRpi->print(F("[DECODERS]:"));
  serialRpi->print(DecoderWorking(1), DEC);
  serialRpi->println(DecoderWorking(2), DEC);

#ifndef OPTION_RASP_WATCH_DOG
  serialRpi->println(F("[WARN] WDT_DISABLED"));
#endif

  if (RpiWtdGetEnabled())
    serialRpi->println(F("[WDT]:ON"));
  else
    serialRpi->println(F("[WDT]:OFF"));

  ConfigPrint();
}

/*****************************************************************************/
void ProtocoloSendRFIDMsg(unsigned char *cardId, unsigned int rfidDataLen)
{
  if (configData.serialProtocolLegacy == 1)
  {
    serialRpi->print(F("[RFID] CARD:"));
    ProtocoloPrintHex(serialRpi, cardId, rfidDataLen);
  }
  else
  {
    SerialProtocolPrintHex(serialRpi, cardId, rfidDataLen, FrameTypeRFID);
  }
}

/*****************************************************************************/
void ProtocoloSendValidatorValue(unsigned char *packet, unsigned int packetLen)
{
  if (configData.serialProtocolLegacy == 1)
  {
    serialRpi->print(F("[VALID] VALUE:"));
    ProtocoloPrintHex(serialRpi, packet, packetLen);
  }
  else
  {
    SerialProtocolPrintHex(serialRpi, packet, packetLen, FrameTypeValidator);
  }
}

/*****************************************************************************/
// comando recebido do validador - referente a rfid comands
// vai do atmega UCP -> main UCP -> main TD -> atmega TD
// manda o pacote que recebeu as-is, menos o header e footer
void ProtocoloSendValidatorToRFID(void *bfr, unsigned int packetLen)
{
  serialRpi->print(F("[UCP2TD] "));
  unsigned char *buffer = (unsigned char *)bfr;
  ProtocoloPrintHex(serialRpi, buffer, packetLen);
}

/*****************************************************************************/
// Resposta da funcao mifare, envia de volta pro validador
// atmega TD -> main TD -> main UCP -> atmega UPP -> validador
void ProtocoloSendRFIDToValidator(void *bfr, unsigned int packetLen)
{
  serialRpi->print(F("[TD2UCP] "));
  unsigned char *buffer = (unsigned char *)bfr;
  ProtocoloPrintHex(serialRpi, buffer, packetLen);
}

/*****************************************************************************/
// no atmega do TD
// recebido pela serial do main TD
// vai executar a funcao mifare
void ProtocoloProcessUCP2TD(unsigned char *bfr, unsigned char packetLen)
{
  // converter de ascci pra bin
  unsigned char output[50];
  memset(output, 0, sizeof(output));
  int len = hexStringToBinary(bfr, packetLen, output);

  // Executa o pacote recebido no validador
  validatorRfidSwitch(output);
}

/*****************************************************************************/
// no atmega da UCP
// recebido pela serial do main UCP
// vai enviar a resposta pro validador
void ProtocoloProcessTD2UCP(unsigned char *bfr, unsigned char packetLen)
{
  // converter de ascci pra bin
  unsigned char output[50];
  memset(output, 0, sizeof(output));
  int len = hexStringToBinary(bfr, packetLen, output);

  // envia pro validador o pacote binario
  ValidatorProtocolDumpHexPacket(output);
}

/*****************************************************************************/
// struct protocolTestStruct {
//   unsigned char var01;
//   unsigned int var02;
//   double var03;
//   float var04;
//   unsigned char crc;
// };
// void ProtocolTest(Stream* serial) {
//   protocolTestStruct dataStruct;

//   unsigned char encodedBuffer[sizeof(protocolTestStruct) * 2];

//   unsigned char decodedBuffer[sizeof(protocolTestStruct) * 2];

//   while (true) {
//     dataStruct.var01 += 1;
//     dataStruct.var02 += 100;
//     dataStruct.var03 += 0.1;
//     dataStruct.var04 += 0.1;

//     dataStruct.crc = ProtocolCalcCRC((void*)&dataStruct, sizeof(protocolTestStruct));

//     unsigned char encodedLen = ProtocolEncodeBuffer((void*)&dataStruct, &encodedBuffer, sizeof(protocolTestStruct));

//     ProtocoloPrintHex(serial, encodedBuffer, encodedLen);

//     protocolTestStruct receivedStruct;

//     unsigned char decodedLen = ProtocolDecodeBuffer(&encodedBuffer, (void*)&receivedStruct, encodedLen);

//     if (ProtocolCalcCRC((void*)&receivedStruct, sizeof(protocolTestStruct)) == receivedStruct.crc)
//       serial->println(F("PACKET OK"));
//     else
//       serial->println(F("PACKET ERROR"));

//     delay(500);
//   }
// }

void SerialProtocolPrintHex(Stream *serial, unsigned char *data, unsigned int length, byte frameType)
{
  Frame frame;

  frame.Id = frameType;
  frame.Sequence = ProtocolSequenceNumber++;
  frame.Length = length;
  // frame.Data = data;

  FrameSerialize(serial, &frame, data);

  delay(10);

  serial->flush();
}

void SerialPrintInt16(Stream *serial, unsigned int value)
{
  for (int i = 12; i >= 0; i -= 4)
  {
    unsigned int digit = (value >> i) & 0xF;
    char c;
    if (digit < 10)
      c = '0' + digit;
    else
      c = 'A' + (digit - 10);

    serial->print(c);
  }
}

void SerialPrintByte(Stream *serial, unsigned char value)
{
  for (int i = 4; i >= 0; i -= 4)
  {
    unsigned char digit = (value >> i) & 0xF;
    char c;

    if (digit < 10)
      c = '0' + digit;
    else
      c = 'A' + (digit - 10);

    serial->print(c);
    // delay(1);
    // serial->flush();
  }
}

void forceReset()
{
  wdt_enable(WDTO_15MS); // Habilita watchdog com timeout mínimo (15ms)
  while (1)
  {
  } // Aguarda o reset
}
