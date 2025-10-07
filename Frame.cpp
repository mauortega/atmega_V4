#include "Arduino.h"
#include "Frame.h"
#include "protocol.h"

Frame::Frame() {
  Id = 0;
  Sequence = 0;
  Length = 0;
  CRC = 0;
}

Frame::~Frame() {
}

void FrameSerialize(Stream* serial, Frame* frame, unsigned char* data) {
  // Id + Sequence + Length + Data + CRC
  int dataSize = 1 + 2 + 2 + frame->Length;

  // DataSize + CRC
  byte buffer[dataSize + 2];

  int offset = 0;

  buffer[offset++] = frame->Id;

  writeInt16BigEndian(buffer, offset, frame->Sequence);
  offset += 2;

  writeInt16BigEndian(buffer, offset, frame->Length);
  offset += 2;

  if (frame->Length > 0) {
    memcpy(buffer + offset, data, frame->Length);
    offset += frame->Length;
  }

  frame->CRC = calculateCRC(buffer, dataSize);

  writeInt16BigEndian(buffer, offset, frame->CRC);

  serial->print('#');

  for (int c = 0; c < dataSize + 2; c++) {
    SerialPrintByte(serial, buffer[c]);
    delayMicroseconds(200);
  }

  serial->println('*');
}

Frame* Frame::deserialize(String frameString) {
  // Verifica se o tamanho mínimo está presente
  if (frameString.length() < 14) {  // 1 byte Id + 2 bytes Sequence + 2 bytes Length + 2 bytes CRC = 7 bytes = 14 caracteres hexadecimais
    return NULL;
  }

  // Separa o CRC da string
  int crcLength = 4;  // 2 bytes em hexadecimal (4 caracteres)
  String dataHex = frameString.substring(0, frameString.length() - crcLength);
  String crcHex = frameString.substring(frameString.length() - crcLength);

  // Converte dataHex em bytes
  int dataLength = dataHex.length() / 2;
  byte* dataBytes = new byte[dataLength];
  for (int i = 0; i < dataLength; i++) {
    String byteString = dataHex.substring(i * 2, i * 2 + 2);
    dataBytes[i] = (byte)strtol(byteString.c_str(), NULL, 16);
  }

  // Converte crcHex em int16_t
  byte crcBytes[2];
  for (int i = 0; i < 2; i++) {
    String byteString = crcHex.substring(i * 2, i * 2 + 2);
    crcBytes[i] = (byte)strtol(byteString.c_str(), NULL, 16);
  }
  int16_t receivedCRC = (crcBytes[0] << 8) | crcBytes[1];

  // Calcula o CRC dos dataBytes
  int16_t calculatedCRC = calculateCRC(dataBytes, dataLength);

  // Compara os CRCs
  if (receivedCRC != calculatedCRC) {
    delete[] dataBytes;
    return NULL;
  }

  // Prossegue com a desserialização
  Frame* frame = new Frame();

  int offset = 0;

  // Lê o Id
  frame->Id = dataBytes[offset++];

  // Lê Sequence em Big-Endian
  frame->Sequence = readInt16BigEndian(dataBytes, offset);
  offset += 2;

  // Lê Length em Big-Endian
  frame->Length = readInt16BigEndian(dataBytes, offset);
  offset += 2;

  // Lê Data
  if (frame->Length > 0) {
    //frame->Data = new byte[frame->Length];
    //memcpy(frame->Data, dataBytes + offset, frame->Length);
  } else {
    //frame->Data = NULL;
  }

  frame->CRC = receivedCRC;

  delete[] dataBytes;

  return frame;
}

int16_t calculateCRC(byte* data, int length) {
  uint16_t crc = 0xFFFF;
  for (int i = 0; i < length; i++) {
    crc ^= (uint16_t)data[i] << 8;
    for (int j = 0; j < 8; j++) {
      if (crc & 0x8000)
        crc = (crc << 1) ^ 0x1021;
      else
        crc <<= 1;
    }
  }
  return crc;
}

void writeInt16BigEndian(byte* buffer, int offset, int16_t value) {
  buffer[offset] = (value >> 8) & 0xFF;
  buffer[offset + 1] = value & 0xFF;
}

int16_t readInt16BigEndian(byte* buffer, int offset) {
  return (int16_t)((buffer[offset] << 8) | buffer[offset + 1]);
}
