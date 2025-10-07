#ifndef rfid_h
#define rfid_h

#include <Arduino.h>

// Polling na leitora
void RfidLoop();

void RFIDSetup(Stream *serial);

// ********************** Aux Functions  **************************
uint32_t GetUint32(unsigned char *src);

void SetUint32(uint32_t value, unsigned char *src);

// ********************** RFID  **************************

unsigned char RFIDPower(unsigned char *src);

void RFIDResetRF();

unsigned char RFIDWriteSector(unsigned char *src);

unsigned char RFIDReadSector(unsigned char *src);

unsigned char RFIDRestore(unsigned char *src);

unsigned char RFIDTransfer(unsigned char *src);

unsigned char RFIDIncrement(unsigned char *src);

////////////////////////////////////////////////////////////////////
unsigned char RFIDDecrement(unsigned char *src);

unsigned char RFIDWriteBlock(unsigned char *src);

////////////////////////////////////////////////////////////////////
unsigned char RFIDReadBlock(unsigned char *src);

void RFIDSetAuth(unsigned char *src);

void RFIDLoadKey(void *src);

void RFIDSelectCard(void *src);

bool RFIDCardPresent();

unsigned char* getRFIDUid();
unsigned char getRFIDUidLength();

unsigned char* rfidUidReaded();

unsigned char* rfidAtqa();

unsigned char rfidSak();

unsigned char* rfidDataBlock();

unsigned char* rfidDataSector();

#endif  // RFID_h
