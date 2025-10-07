#ifndef VALIDATOR_H
#define VALIDATOR_H

// Revisar isso
#define VALIDATOR_RS485_RXTX A3

void ValidatorSetTransmitionMode();
void ValidatorSetReceptionMode();

void ValidatorSetup(Stream &debugSerial);
void ValidatorLoop();

bool ValidatorStdPacketReceived();
bool ValidatorRfidPacketReceived();
bool ValidatorUcpPacketReceived();
bool ValidatorTransdataPacketReceived();

void ValidatorStdPacketReceivedReset();
void ValidatorRfidPacketReceivedReset();
void ValidatorUcpPacketReceivedReset();
void ValidatorTransdataPacketReceivedReset();

unsigned char *ValidatorGetPacket();
unsigned int ValidatorPacketLen();

void ValidatorSimulator();
byte ValidatorCRC(void *data, int len);
void ValidatorTestPacket(Stream *serial);

#endif
