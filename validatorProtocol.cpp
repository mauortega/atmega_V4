#include "validatorProtocol.h"

#include <Arduino.h>
#include "app.h"
#include "protocol.h"
#include "validator.h"
#include "validatorRfid.h"
#include "config.h"

// const unsigned char ProdataTestPacket[] = {0x02, 0xEA, 0x02, 0x00, 0x00, 0x03, 0xEB};
// const unsigned char ProdataUcpTestPacket[] = {0xAA, 0xC9, 0x00, 0x01, 0x02, 0x55, 0x35};

static Stream *serialValidator;
static Stream *serialRpi;

bool ValidatorProtocolStarted = false;

////////////////////////////////////////////////////
void ValidatorProtocolSetup(Stream &protocol_port, Stream &debug_port)
{
  serialValidator = &protocol_port;
  serialRpi = &debug_port;

  //   pinMode(PRODATA_RS485_RXTX, OUTPUT);
  //   ValidatorProtocolSetReceptionMode();

  ValidatorProtocolStarted = true;
}

////////////////////////////////////////////////////
// main loop, sem timer
void ValidatorProtocolProcess()
{
  // Protocolo standard
  if (ValidatorStdPacketReceived())
  {
    ValidatorStdPacketReceivedReset();
    unsigned char *packet = ValidatorGetPacket();
    unsigned int packetLen = ValidatorPacketLen();

    if (packet[1] == PRODATA_V2_TYPE_POOLING)
    {
      unsigned char sequence = 0;
      if (packet[3] == PRODATA_STD_ESCAPE_ID)
      {
        sequence = packet[4] - 0x20;
      }
      else
      {
        sequence = packet[3];
      }

      unsigned char dest[50];
      unsigned int size = ValidatorProtocolV2Build(nullptr, dest, 0, PRODATA_V2_ACK_POOLING, PRODATA_V2_VERSION, sequence);

      ValidatorProtocolSendPacket(dest, size);
    }
    else if (packet[1] == PRODATA_V2_TYPE_OPERATION)
    {
      unsigned char sequence = 0;
      if (packet[3] == PRODATA_STD_ESCAPE_ID)
      {
        sequence = packet[4] - 0x20;
      }
      else
      {
        sequence = packet[3];
      }

      unsigned char dest[50];
      unsigned int size = ValidatorProtocolV2Build(nullptr, dest, 0, PRODATA_V2_ACK_OPERATION, PRODATA_V2_VERSION, sequence);

      ValidatorProtocolSendPacket(dest, size);
    }
    else if (packet[1] == PRODATA_V2_TYPE_PASSANGER)
    {
      unsigned char sequence = 0;
      if (packet[3] == PRODATA_STD_ESCAPE_ID)
      {
        sequence = packet[4] - 0x20;
      }
      else
      {
        sequence = packet[3];
      }

      unsigned char dest[50];
      unsigned int size = ValidatorProtocolV2Build(nullptr, dest, 0, PRODATA_V2_ACK_PASSANGER, PRODATA_V2_VERSION, sequence);

      ValidatorProtocolSendPacket(dest, size);
    }

    ProtocoloSendValidatorValue(packet, packetLen);
  }

  // Comandos da catraca - controla leitora rfid
  if (ValidatorRfidPacketReceived())
  {
    ValidatorRfidPacketReceivedReset();
    unsigned char *pckt = (unsigned char *)ValidatorGetPacket();

    if (configData.validatorRfidEnabled == 1 && configData.validatorRfidDirect == 1)
    {
      validatorRfidSwitch(pckt);
    }
    else if (configData.validatorRfidEnabled == 1 && configData.validatorRfidDirect == 0)
    {
      ProtocoloSendValidatorToRFID(pckt, ValidatorPacketLen());
    }
  }

  // protocolo td prodata
  if (ValidatorUcpPacketReceived())
  {
    ValidatorUcpPacketReceivedReset();

    if (configData.validatorRfidEnabled == 1) {
      ProdataUCPSendAck();
    }
  }

  if (ValidatorTransdataPacketReceived())
  {
    ValidatorTransdataPacketReceivedReset();
    unsigned char *packet = ValidatorGetPacket();
    unsigned int packetLen = ValidatorPacketLen();

    ProtocoloSendValidatorValue(packet, packetLen);
  }
}

//-------------------------------------------------------------------------------
// put entire packet here
// call this function to validade the received packet
/*
bool ValidatorProtocolPacketCheck(void *src, int len) {
  // TODO: check packet length?

  unsigned char *buffer = (unsigned char *)src;

  ValidatorProtocolPrintPacket(src, *serialRpi);

  unsigned char type = ValidatorProtocolGetType(buffer);

  if (type == PRODATA_STX_ID) {

    if (PRODATA_ETX(buffer) != PRODATA_ETX_ID)
      return false;

    // ValidatorProtocolPrintDescription(buffer, *serialRpi);

    unsigned char test_crc = PRODATA_CRC(buffer);

    // from the 2nd byte to ETX byte
    for (int i = 1; i <= 3 + PRODATA_LENGTH(buffer); i++)
      test_crc ^= buffer[i];

    //serialRpi->print("check: ");

    if (test_crc != 0) {
      serialRpi->print("FAIL ");
      serialRpi->println(test_crc, HEX);  // PROD_local???
    }

    serialRpi->println();

    return test_crc == 0;
  }
  else if (type == PRODATA_UCP_STX_ID) {

    if (PRODATA_UCP_ETX(buffer) != PRODATA_UCP_ETX_ID)
      return false;

    // ValidatorProtocolPrintDescription(buffer, *serialRpi);

    unsigned char test_crc = PRODATA_UCP_CRC(buffer);

    // from the 2nd byte to ETX byte
    for (int i = 0; i <= 4 + PRODATA_UCP_LENGTH(buffer); i++)
      test_crc ^= buffer[i];

    //serialRpi->print("check: ");

    if (test_crc != 0) {
      serialRpi->print("FAIL ");
      serialRpi->println(test_crc, HEX);  // PROD_local???
    }

    return test_crc == 0;
  }

  serialRpi->println("PROTOCOL FAIL");
  return false;
}

*/

/*
/// @brief /////////////////////////////////////
/// @param src
/// @param serial_port /
void ValidatorProtocolPrintDescription(void *src, Stream &serial_port) {
  unsigned char *buffer = (unsigned char *)src;

  unsigned char type = ValidatorProtocolGetType(buffer);

  if (type == PRODATA_STX_ID) {
    serial_port.println("PRODATA TYPE");

    serial_port.print("cmd: ");

    if (PRODATA_CMD(buffer) < 0x10)
      serial_port.print("0");

    serial_port.println(PRODATA_CMD(buffer), HEX);

    serial_port.print("len: ");
    serial_port.println(PRODATA_LENGTH(buffer));

    serial_port.print("crc: ");

    if (PRODATA_CRC(buffer) < 0x10)
      serial_port.print("0");

    serial_port.println(PRODATA_CRC(buffer), HEX);
  }
  else if (type == PRODATA_UCP_STX_ID) {
    serial_port.println("UCP TYPE");

    serial_port.print("cmd: ");

    if (PRODATA_UCP_CMD(buffer) < 0x10)
      serial_port.print("0");

    serial_port.println(PRODATA_UCP_CMD(buffer), HEX);

    serial_port.print("len: ");
    serial_port.println(PRODATA_UCP_LENGTH(buffer));

    serial_port.print("crc: ");

    if (PRODATA_UCP_CRC(buffer) < 0x10)
      serial_port.print("0");

    serial_port.println(PRODATA_UCP_CRC(buffer), HEX);
  }
  else {
    serial_port.println("*protocol not recognized*");
    return;
  }
}
*/

/// @brief //////////////////////////////////////////////
/// @param src
/// @param serial_port
// void ValidatorProtocolPrintPacket(void *src, Stream &serial_port)
// {
//   unsigned char *inbuffer = (unsigned char *)src;

//   int len;
//   serial_port.print("[DEBUG_VALID] ");

//   if (PRODATA_STX(inbuffer) == PRODATA_STX_ID)
//   {
//     len = PRODATA_PACKET_LENGTH(inbuffer);
//   }
//   else if (PRODATA_UCP_STX(inbuffer) == PRODATA_UCP_STX_ID)
//   {
//     len = PRODATA_UCP_PACKET_LENGTH(inbuffer);
//   }
//   else
//   {
//     serial_port.println(F("[ERR] *protocol not recognized*"));
//     return;
//   }

//   for (int i = 0; i < len; i++)
//   {
//     if (inbuffer[i] < 0x10)
//       serial_port.print("0");
//     serial_port.print(inbuffer[i], HEX);
//     serial_port.print(" ");
//   }
//   serial_port.println();
// }

////////////////////////////////////////////////////
void ValidatorProtocolDumpHexPacket(void *src)
{
  unsigned char *buffer = (unsigned char *)src;

  ValidatorSetTransmitionMode();
  serialValidator->write(buffer, PRODATA_PACKET_LENGTH(buffer));
  serialValidator->flush();
  ValidatorSetReceptionMode();
}

void ValidatorProtocolSendPacket(void *src, int len)
{
  unsigned char *buffer = (unsigned char *)src;

  ValidatorSetTransmitionMode();
  serialValidator->write(buffer, len);
  serialValidator->flush();
  ValidatorSetReceptionMode();
}

////////////////////////////////////////////////////
int ValidatorProtocolBuild(void *src, void *dest, unsigned char data_len, unsigned char cmd)
{
  unsigned char *inbuffer = (unsigned char *)src;
  unsigned char *outbuffer = (unsigned char *)dest;

  PRODATA_STX(outbuffer) = PRODATA_STX_ID;
  PRODATA_CMD(outbuffer) = cmd;
  PRODATA_LENGTH(outbuffer) = data_len;

  unsigned char crc = cmd ^ data_len;

  for (int i = 0; i < data_len; i++)
  {
    PRODATA_DATA(outbuffer, i) = inbuffer[i];
    crc ^= inbuffer[i];
  }

  PRODATA_ETX(outbuffer) = PRODATA_ETX_ID;
  crc ^= PRODATA_ETX_ID;

  PRODATA_CRC(outbuffer) = crc;

  return PRODATA_PACKET_LENGTH(outbuffer);
}

////////////////////////////////////////////////////
int ValidatorProtocolV2Build(void *src, void *dest, unsigned int data_len, unsigned char cmd, unsigned char version, unsigned char sequence)
{
  unsigned char *inbuffer = (unsigned char *)src;
  unsigned char *outbuffer = (unsigned char *)dest;

  unsigned int index = 0;

  outbuffer[index++] = PRODATA_STD_STX_ID;
  outbuffer[index++] = cmd;
  outbuffer[index++] = version;

  if (sequence == 0x01 || sequence == 0x04 || sequence == 0x10 || sequence == 0x11 || sequence == 0x13)
  {
    outbuffer[index++] = 0x10;
    outbuffer[index++] = sequence + 0x20;
  }
  else
  {
    outbuffer[index++] = sequence;
  }

  unsigned char byte1 = data_len & 0xFF;
  unsigned char byte2 = (data_len >> 8) & 0xFF;

  // atmega is little-endian, inverting for big-endian protocol
  if (byte2 == 0x01 || byte2 == 0x04 || byte2 == 0x10 || byte2 == 0x11 || byte2 == 0x13)
  {
    outbuffer[index++] = 0x10;
    outbuffer[index++] = byte2 + 0x20;
  }
  else
  {
    outbuffer[index++] = byte2;
  }

  if (byte1 == 0x01 || byte1 == 0x04 || byte1 == 0x10 || byte1 == 0x11 || byte1 == 0x13)
  {
    outbuffer[index++] = 0x10;
    outbuffer[index++] = byte1 + 0x20;
  }
  else
  {
    outbuffer[index++] = byte1;
  }

  for (int i = 0; i < data_len; i++)
  {
    if (inbuffer[i] == 0x01 || inbuffer[i] == 0x04 || inbuffer[i] == 0x10 || inbuffer[i] == 0x11 || inbuffer[i] == 0x13)
    {
      outbuffer[index++] = 0x10;
      outbuffer[index++] = inbuffer[i] + 0x20;
    }
    else
    {
      outbuffer[index++] = inbuffer[i];
    }
  }

  unsigned char crc = 0;

  for (int i = 1; i < index; i++)
  {
    crc += outbuffer[i];
  }

  if (crc == 0x01 || crc == 0x04 || crc == 0x10 || crc == 0x11 || crc == 0x13)
  {
    outbuffer[index++] = 0x10;
    outbuffer[index++] = crc + 0x20;
  }
  else
  {
    outbuffer[index++] = crc;
  }

  outbuffer[index++] = PRODATA_STD_ETX_ID;

  return index;
}

//-------------------------------------------------------------------------------
// returns the length of the destinationa array
//-------------------------------------------------------------------------------
// int ValidatorProtocolUcpBuild(void *src, void *dest, unsigned char data_len, unsigned char cmd)
// {
//   unsigned char *inbuffer = (unsigned char *)src;
//   unsigned char *outbuffer = (unsigned char *)dest;

//   int index = 0;

//   unsigned char vaux;
//   // vaux = 0x00;  // as per protocol PDF

//   outbuffer[index++] = 0xAA; // STX
//   // vaux ^= 0xAA;
//   vaux = 0xAA;

//   outbuffer[index++] = cmd; // CMD
//   vaux ^= cmd;

//   outbuffer[index++] = 0x00; // LEN MSB = 0x00, always
//   // vaux ^= 0x00; // not needed

//   outbuffer[index++] = data_len; // LEN LSB = actual length
//   vaux ^= data_len;

//   for (int i = 0; i < data_len; i++)
//   {
//     outbuffer[index++] = inbuffer[i];
//     vaux ^= inbuffer[i];
//   }

//   // CRC
//   outbuffer[index++] = vaux;

//   return index;
// }

/////////////////////////////////////////////////////////////////

void ValidatorProtocolCardDetected()
{
  const unsigned char UcpCardPresent[] = {0xAA, 0xB1, 0x00, 0x01, 0xFE, 0x55, 0xB1};

  ValidatorSetTransmitionMode();
  serialValidator->write(UcpCardPresent, 7);
  serialValidator->flush();
  ValidatorSetReceptionMode();
}

//-------------------------------------------------------------------------------
void ProdataUCPSendAck()
{
  const unsigned char ProdataUcpAck[] = {0xAA, 0xAA};

  ValidatorSetTransmitionMode();
  delay(1);
  serialValidator->write(ProdataUcpAck, 2);
  serialValidator->flush();
  ValidatorSetReceptionMode();
}
