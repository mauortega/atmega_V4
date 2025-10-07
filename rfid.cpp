
#include <Arduino.h>

#include "rfid.h"
#include "app.h"
#include "config.h"
#include "validatorProtocol.h"
#include "protocol.h"

#include <SPI.h>
#include <PN532_SPI.h>
#include <PN532.h>

#define RF_CS_PIN 10
#define RF_RST_PIN 9

PN532_SPI pn532spi(SPI, RF_CS_PIN);
PN532 nfc(pn532spi);

// ********************** RFID  **************************

static Stream *serialRpi;

#define IS_KEYA (0x60)
#define IS_KEYB (0x61)

#define IS_SINGLE (0x26)
#define IS_ALL (0x52)

unsigned char RFIDRequestType = IS_SINGLE;

unsigned char RFIDKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
unsigned char RFIDDataBlock[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned char RFIDDataSector[48] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

unsigned char RFIDBlockAddress = 0;

unsigned char RFIDAuthType = IS_KEYA;

unsigned char RFIDUid[] = {0, 0, 0, 0, 0, 0, 0};
unsigned char RFIDUidReaded[] = {0, 0, 0, 0, 0, 0, 0};
unsigned char RFIDUidSelected[] = {0, 0, 0, 0, 0, 0, 0};

// Length of the UID (4 or 7 bytes depending on ISO14443A card type)
unsigned char RFIDUidLength;

unsigned char RFIDAtqa[2];
unsigned char RFIDSak;

#define RFID_TIMEOUT 5000
static unsigned long timeout = 0;

////////////////////////////////////////////////////////////////////////

void RfidSetTimeout() 
{
  timeout = millis();
}

////////////////////////////////////////////////////////////////////////

void RfidLoop()
{
  if (timeout > 0 && (millis() - timeout) < RFID_TIMEOUT)
  {
    return;
  }
  
  // serialRpi->println(F("Scanning for cards..."));

  timeout = 0;

  RFIDSetup(serialRpi);

  if (RFIDCardPresent())
  {
    unsigned char *cardId = getRFIDUid();
    unsigned char rfidDataLen = getRFIDUidLength();

    RfidSetTimeout();

    if (configData.validatorRfidEnabled == 1 && configData.validatorRfidDirect == 1)
    {
      ValidatorProtocolCardDetected();
    }

    ProtocoloSendRFIDMsg(cardId, rfidDataLen);
  }
}

////////////////////////////////////////////////////////////////////////
void RFIDSetup(Stream *serial)
{
  serialRpi = serial;

  SPI.beginTransaction(SPISettings(400000, MSBFIRST, SPI_MODE0));

  RFIDResetRF();

  nfc.begin();
  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  nfc.setPassiveActivationRetries(0x09);

  // configure board to read RFID tags
  nfc.SAMConfig();
}

////////////////////////////////////////////////////////////////////////
unsigned char RFIDPower(unsigned char *src)
{
  unsigned char *_temp = (unsigned char *)src;

  unsigned char state = _temp[0];

  return 1;
}

void RFIDResetRF()
{
  // digitalWrite(RF_RST_PIN, HIGH);
  // delay(10);
  digitalWrite(RF_RST_PIN, LOW);
  delay(10);
  digitalWrite(RF_RST_PIN, HIGH);
  delay(10);
  digitalWrite(RF_RST_PIN, LOW);
}

unsigned char RFIDWriteSector(unsigned char *src)
{
  unsigned char *_temp = (unsigned char *)src;
  uint8_t success;

  RfidSetTimeout();

  RFIDBlockAddress = _temp[0] * 4;

  for (int i = 1; i < 49; i++)
    RFIDDataSector[i - 1] = _temp[i];

  unsigned char keyNumber = 0;

  if (RFIDAuthType == IS_KEYB)
    keyNumber = 1;

  for (unsigned char block = 0; block < 3; block++)
  {
    unsigned char blockAddr = RFIDBlockAddress + block;

    success = nfc.mifareclassic_AuthenticateBlock(RFIDUid, RFIDUidLength, blockAddr, keyNumber, RFIDKey);

    if (success)
    {
      // Try to write the contents of block
      success = nfc.mifareclassic_WriteDataBlock(blockAddr, &RFIDDataSector[block * 16]);

      if (!success)
        return 1; // unable to read the requested block

      return 0; // OK
    }

    return 2; // authentication failed
  }
}

unsigned char RFIDReadSector(unsigned char *src)
{
  unsigned char *_temp = (unsigned char *)src;
  uint8_t success;

  RfidSetTimeout();

  RFIDBlockAddress = _temp[0] * 4;

  unsigned char keyNumber = 0;
  if (RFIDAuthType == IS_KEYB)
    keyNumber = 1;

  for (unsigned char block = 0; block < 3; block++)
  {
    unsigned char blockAddr = RFIDBlockAddress + block;

    success = nfc.mifareclassic_AuthenticateBlock(RFIDUid, RFIDUidLength, blockAddr, keyNumber, RFIDKey);

    if (success)
    {
      // Try to write the contents of block
      success = nfc.mifareclassic_ReadDataBlock(blockAddr, RFIDDataBlock);

      if (!success)
        return 1; // unable to read the requested block

      memcpy(&RFIDDataSector[block * 16], RFIDDataBlock, 16);
      return 0; // OK
    }

    return 2; // authentication failed
  }
}

unsigned char RFIDRestore(unsigned char *src)
{
  unsigned char *_temp = (unsigned char *)src;

  RFIDBlockAddress = _temp[0];

  return 2;
}

unsigned char RFIDTransfer(unsigned char *src)
{
  unsigned char *_temp = (unsigned char *)src;

  RFIDBlockAddress = _temp[0];

  return 2;
}

unsigned char RFIDIncrement(unsigned char *src)
{
  unsigned char *_temp = (unsigned char *)src;
  uint8_t success;
  uint32_t value;

  RFIDBlockAddress = _temp[0];

  value = GetUint32(&_temp[1]);

  return 2;
}

////////////////////////////////////////////////////////////////////
unsigned char RFIDDecrement(unsigned char *src)
{
  unsigned char *_temp = (unsigned char *)src;
  uint8_t success;
  uint32_t value;

  RFIDBlockAddress = _temp[0];

  value = GetUint32(&_temp[1]);

  return 2;
}

unsigned char RFIDWriteBlock(unsigned char *src)
{
  unsigned char *_temp = (unsigned char *)src;
  uint8_t success;

  RfidSetTimeout();

  RFIDBlockAddress = _temp[0];

  for (int i = 1; i < 17; i++)
    RFIDDataBlock[i - 1] = _temp[i];

  unsigned char keyNumber = 0;

  if (RFIDAuthType == IS_KEYB)
    keyNumber = 1;

  success = nfc.mifareclassic_AuthenticateBlock(RFIDUid, RFIDUidLength, RFIDBlockAddress, keyNumber, RFIDKey);

  if (success)
  {
    // Try to write the contents of block
    success = nfc.mifareclassic_WriteDataBlock(RFIDBlockAddress, RFIDDataBlock);

    if (!success)
      return 1; // unable to read the requested block

    return 0; // OK
  }

  return 2; // authentication failed
}

////////////////////////////////////////////////////////////////////
unsigned char RFIDReadBlock(unsigned char *src)
{
  unsigned char *_temp = (unsigned char *)src;
  uint8_t success;

  RfidSetTimeout();

  RFIDBlockAddress = _temp[0];

  unsigned char keyNumber = 0;

  if (RFIDAuthType == IS_KEYB)
    keyNumber = 1;

  /*!
    Tries to authenticate a block of memory on a MIFARE card using the
    INDATAEXCHANGE command.  See section 7.3.8 of the PN532 User Manual
    for more information on sending MIFARE and other commands.

    @param  uid           Pointer to a byte array containing the card UID
    @param  uidLen        The length (in bytes) of the card's UID (Should
                          be 4 for MIFARE Classic)
    @param  blockNumber   The block number to authenticate.  (0..63 for
                          1KB cards, and 0..255 for 4KB cards).
    @param  keyNumber     Which key type to use during authentication
                          (0 = MIFARE_CMD_AUTH_A, 1 = MIFARE_CMD_AUTH_B)
    @param  keyData       Pointer to a byte array containing the 6 bytes
                          key value

    @returns 1 ifeverything executed properly, 0 for an error
*/

  success = nfc.mifareclassic_AuthenticateBlock(RFIDUid, RFIDUidLength, RFIDBlockAddress, keyNumber, RFIDKey);

  if (success)
  {
    // Try to read the contents of block
    success = nfc.mifareclassic_ReadDataBlock(RFIDBlockAddress, RFIDDataBlock);

    if (!success)
      return 1; // unable to read the requested block

    return 0; // OK
  }

  return 2; // authentication failed
}

void RFIDSetAuth(unsigned char *src)
{
  unsigned char *_temp = (unsigned char *)src;

  RFIDAuthType = _temp[0];
}

void RFIDLoadKey(void *src)
{
  unsigned char *_temp = (unsigned char *)src;

  for (int i = 0; i < 6; i++)
    RFIDKey[i] = _temp[i];
}

void RFIDSelectCard(void *src)
{
  unsigned char *_temp = (unsigned char *)src;

  for (int i = 0; i < 4; i++)
    RFIDUidSelected[i] = _temp[i];
}

bool RFIDCardPresent()
{
  digitalWrite(RF_CS_PIN, HIGH);

  bool success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, RFIDUid, &RFIDUidLength);

  if (success)
  {
    unsigned char len;
    unsigned char *packetbuffer = nfc.getBuffer(&len);
    RFIDAtqa[0] = packetbuffer[3]; // ATQA: The card's identification (typically 2 bytes).
    RFIDAtqa[1] = packetbuffer[2]; // ATQA: The card's identification (typically 2 bytes).
    RFIDSak = packetbuffer[4];     // SAK: The Select Acknowledge byte (1 byte) that indicates the card type and capabilities.
    RfidSetTimeout();
  }

  digitalWrite(RF_CS_PIN, LOW);

  return success;
}
// serial number
unsigned char *getRFIDUid()
{
  return RFIDUid;
}

unsigned char getRFIDUidLength()
{
  return RFIDUidLength;
}

unsigned char *rfidUidReaded()
{
  return RFIDUidReaded;
}

unsigned char *rfidAtqa()
{
  return RFIDAtqa;
}

unsigned char rfidSak()
{
  return RFIDSak;
}

unsigned char *rfidDataBlock()
{
  return RFIDDataBlock;
}

unsigned char *rfidDataSector()
{
  return RFIDDataSector;
}

// ********************** Aux Functions  **************************
uint32_t GetUint32(unsigned char *src)
{
  unsigned char *_temp = (unsigned char *)src;

  uint32_t value =
      (uint32_t)_temp[0] + 0x100 * (uint32_t)_temp[1] + 0x10000 * (uint32_t)_temp[2] + 0x1000000 * (uint32_t)_temp[3];

  return value;
}

void SetUint32(uint32_t value, unsigned char *src)
{
  unsigned char *_temp = (unsigned char *)src;

  for (unsigned char i = 0; i < 4; i++)
  {
    _temp[i] = value % 256;
    value = value / 256;
  }
}
