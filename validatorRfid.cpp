#include "validatorRfid.h"

#include <Arduino.h>
#include "app.h"
#include "protocol.h"
#include "validator.h"
#include "validatorProtocol.h"
#include "rfid.h"
#include <SoftwareSerial.h>
#include "config.h"
//-------------------------------------------------------------------------------

//// resposta:
//     // in case of success
//  ValidatorProtocolBuild({}, buffer, 0, PRODATA_CMD(buffer));

//-------------------------------------------------------------------------------
static Stream *serialRpi;

void ValidatorRfidSetup(Stream &debug_port) {
  serialRpi = &debug_port;
}

void ValidatorSend(void *buffer, int len) {

  if (configData.validatorRfidEnabled == 1 && configData.validatorRfidDirect == 1)
  {
    ValidatorProtocolDumpHexPacket(buffer);
  }
  else if (configData.validatorRfidEnabled == 1 && configData.validatorRfidDirect == 0)
  {
    ProtocoloSendRFIDToValidator(buffer, len);
  }
}

//	0x31	Mifare request – requisição de cartão em idle state, LEN = 0x01, DATA = 0x26 request idle state, DATA = 0x52 request all cards.
void MifareRequest(void *bfr) {
  //serialRpi->println("MifareRequest");

  unsigned char *buffer = (unsigned char *)bfr;

  int len;
  if (RFIDCardPresent()) {
    len = ValidatorProtocolBuild(rfidAtqa(), buffer, 2, PDT_STATUS_MIFARE_OK);
  } else {
    len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_RF_NO_CARD);
  }

  ValidatorSend(buffer, len);  
}

//	0x32	Mifare anticolision – inicia processo de anticolisão
void MifareAnticolision(void *bfr) {
  //serialRpi->println("MifareAnticolision");

  unsigned char *buffer = (unsigned char *)bfr;
  int len;

  if (RFIDCardPresent()) {

    unsigned char* rfiduid=getRFIDUid();
    unsigned char* rfiduidreaded=rfidUidReaded();

    for (int i = 0; i < 4; i++)
      rfiduidreaded[i] = rfiduid[3 - i];

    len = ValidatorProtocolBuild(getRFIDUid(), buffer, 4, PDT_STATUS_MIFARE_OK);
  } else {
    len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_RF_NO_CARD);
  }

  ValidatorSend(buffer, len);
}

//	0x33	Mifare select – seleciona cartão, LEN = 0x04, DATA = card serial number, DATA (byte 6) = MSB, DATA (byte 3) = LSB
void MifareSelect(void *bfr) {
  //serialRpi->println("MifareSelect");

  unsigned char *buffer = (unsigned char *)bfr;
  int len;
  if (RFIDCardPresent()) {
    unsigned char sak1[] = { rfidSak() };
    len = ValidatorProtocolBuild(sak1, buffer, 1, PDT_STATUS_MIFARE_OK);
  } else {
    len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_RF_NO_CARD);
  }
  
  ValidatorSend(buffer, len);
}

//	0x34	Mifare load key – carrega 6 bytes, chave de autenticação. LEN = 0x06, DATA (byte 3) = MSB, DATA (byte 8) = LSB
void MifareLoadKey(void *bfr) {
  //serialRpi->println("MifareLoadKey");

  unsigned char *buffer = (unsigned char *)bfr;
  int len;
  if (RFIDCardPresent()) {
    RFIDLoadKey(&buffer[3]);
    len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_OK);
  } else {
    len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_RF_NO_CARD);
  }

  ValidatorSend(buffer, len);
}

//	0x35	Mifare authentication – comando de autenticação – LEN = 0x06, se DATA (byte 3) = 0x60 chave A, se DATA (byte 3) = 0x61 chave B
void MifareAuthentication(void *bfr) {
  //serialRpi->println("MifareAuthentication");

  unsigned char *buffer = (unsigned char *)bfr;
  int len;
  if (RFIDCardPresent()) {
    RFIDSetAuth(&buffer[3]);
    len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_OK);
  } else {
    len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_RF_NO_CARD);
  }
  
  ValidatorSend(buffer, len);
}

//	0x36	Mifare read – comando de leitura – LEN = 0x01, DATA = block address
void MifareRead(void *bfr) {
  //serialRpi->println("MifareRead");

  unsigned char *buffer = (unsigned char *)bfr;
  int len;

  if (RFIDCardPresent()) {
    unsigned char result = RFIDReadBlock(&buffer[3]);
    unsigned char* data_read = rfidDataBlock();

    if (result == 0) {
      len = ValidatorProtocolBuild(data_read, buffer, 16, PDT_STATUS_MIFARE_OK);
    }
    else {
      len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_ACCESS_ERROR);
    }
  } else {
    len =  ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_RF_NO_CARD);
  }

  ValidatorSend(buffer, len);
}

//	0x37	Mifare write – comando de escrita – LEN = 0x11, DATA (byte 3) = block address, DATA (byte 4 - 19) Dados a serem escritos, byte 4 MSB
void MifareWrite(void *bfr) {
  // serialRpi->println("MifareWrite");

  unsigned char *buffer = (unsigned char *)bfr;
  int len;

  if (RFIDCardPresent()) {
    unsigned char result = RFIDWriteBlock(&buffer[3]);
    if (result == 0) {
      len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_OK);
    } 
    else {
      len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_ACCESS_ERROR);
    }
  } else {
    len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_RF_NO_CARD);
  }

  ValidatorSend(buffer, len);
}

//	0x38	Mifare halt – comando para parar o processo
void MifareHalt(void *bfr) {
  //serialRpi->println("MifareHalt");

  unsigned char *buffer = (unsigned char *)bfr;

  int len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_OK);

  ValidatorSend(buffer, len);
}

//	0x39	Mifare decrement - comando de decremento – LEN = 0x05, DATA (byte 3) = block address (valor maximo de 63), converte DATA (byte 4 a 7) em um valor inteiro de 32 bits sendo byte 4 LSB e byte 7 MSB.
void MifareDecrement(void *bfr) {
  //serialRpi->println("MifareDecrement");

  unsigned char *buffer = (unsigned char *)bfr;
  int len;
  if (RFIDCardPresent()) {
    unsigned char result = RFIDDecrement(&buffer[3]);
    if (result == 0) {
      len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_OK);
    }
    else {
      len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_VALUE_ERROR);
    }
  } else {
    len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_RF_NO_CARD);
  }

  ValidatorSend(buffer, len);
}

//	0x3A	Mifare increment – comando de incremento - LEN = 0x05, DATA (byte 3) = block address (valor maximo de 63), converte DATA (byte 4 a 7) em um valor inteiro de 32 bits sendo byte 4 LSB e byte 7 MSB.
void MifareIncrement(void *bfr) {
  //serialRpi->println("MifareIncrement");

  unsigned char *buffer = (unsigned char *)bfr;
  int len;
  if (RFIDCardPresent()) {
    unsigned char result = RFIDIncrement(&buffer[3]);
    if (result == 0) {
      len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_OK);
    }
    else {
      len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_VALUE_ERROR);
    }
  } else {
    len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_RF_NO_CARD);
  }

  ValidatorSend(buffer, len);
}

//	0x3B	Mifare transfer – transfere valor – LEN = 0x01, DATA = block address (valor maximo de 63)
void MifareTransfer(void *bfr) {
  //serialRpi->println("MifareTransfer");

  unsigned char *buffer = (unsigned char *)bfr;
  int len;
  if (RFIDCardPresent()) {
    unsigned char result = RFIDTransfer(&buffer[3]);
    if (result == 0) {
      len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_OK);
    }
    else {
      len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_VALUE_ERROR);
    }
  } else {
    len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_RF_NO_CARD);
  }

  ValidatorSend(buffer, len);
}

//	0x3C	Mifare restore – comando restaurar - LEN = 0x01, DATA = block address (valor maximo de 63)
void MifareRestore(void *bfr) {
  //serialRpi->println("MifareRestore");

  unsigned char *buffer = (unsigned char *)bfr;
  int len;
  if (RFIDCardPresent()) {
    unsigned char result = RFIDRestore(&buffer[3]);
    if (result == 0) {
      len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_OK);
    } 
    else {
      len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_VALUE_ERROR);
    }
  } else {
    len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_RF_NO_CARD);
  }

  ValidatorSend(buffer, len);
}

//	0x3D	Mifare read sector – lê setor - LEN = 0x01, DATA = sector address (valor maximo de 15) Primeiro bloco lido = DATA (byte 3) multiplicado por 4, 16 bytes – resposta DATA[3 ao 18] Segundo bloco lido = DATA (byte 3+1) multiplicado por 4, 16 bytes – resposta DATA[19 ao 34] Terceiro bloco lido = DATA (byte 3) multiplicado por 4, 16 bytes – resposta DATA[35 ao 50]
void MifareReadSector(void *bfr) {
  //serialRpi->println("MifareReadSector");

  unsigned char data_read[48];
  unsigned char *buffer = (unsigned char *)bfr; 
  int len;
  if (RFIDCardPresent()) {
    unsigned char result = RFIDReadSector(&data_read[0]);
    unsigned char* data_read = rfidDataSector();
    if (result == 0) {
      len = ValidatorProtocolBuild(data_read, buffer, 48, PDT_STATUS_MIFARE_OK);
    }
    else {
      len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_ACCESS_ERROR);
    }
  } else {
    len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_RF_NO_CARD);
  }

  ValidatorSend(buffer, len);
}

//	0x3E	Mifare write sector – escreve em setor - LEN = 0x31, DATA (byte 3) multiplicado por 4 = sector address do primeiro bloco (valor maximo de 15 do byte 3), DATA (byte 4) = MSB first block, DATA (byte 20) = MSB second block, DATA (byte 36) = MSB third block
void MifareWriteSector(void *bfr) {
// serialRpi->println("MifareWriteSector");

  unsigned char *buffer = (unsigned char *)bfr;
  int len;
  if (RFIDCardPresent()) {
    unsigned char result = RFIDWriteSector(&buffer[3]);
    if (result == 0) {
      len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_OK);
    } 
    else {
      len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_ACCESS_ERROR);
    }
  } else {
    len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_RF_NO_CARD);
  }

  ValidatorSend(buffer, len);
}

//	0x3F	Mifare reset RF – reseta o radio
void MifareResetRF(void *bfr) {
 // serialRpi->println("MifareResetRF");

  RFIDResetRF();

  unsigned char *buffer = (unsigned char *)bfr;
  int len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_OK);

  ValidatorSend(buffer, len);
}

//	0x40	Mifare power down – desliga o radio - LEN = 0x01, se DATA = 0x01 desliga, se DATA = 0x00 liga
void MifarePowerDown(void *bfr) {
  //serialRpi->println("MifarePowerDown");
  unsigned char *buffer = (unsigned char *)bfr;

  unsigned char result = RFIDPower(&buffer[3]);
  int len;
  if (result == 0) {
    len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_OK);
  } 
  else {
    len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_INVALID_CMD);
  }

  ValidatorSend(buffer, len);
}

void MifareNotImplemented(void *bfr) {
  //serialRpi->println("MifareNotImplemented");

  unsigned char *buffer = (unsigned char *)bfr;

  int len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_OK);

  ValidatorSend(buffer, len);
}

void MifareSerial(void *bfr) {
 // serialRpi->println("MifareSerial");

  unsigned char *buffer = (unsigned char *)bfr;
  unsigned char number[] = { 0x12, 0x34, 0x56, 0x78 };

  // ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_INVALID_CMD);
  int len = ValidatorProtocolBuild(number, buffer, 4, PDT_STATUS_MIFARE_OK);

  ValidatorSend(buffer, len);
}

void MifareVersion(void *bfr) {
  //serialRpi->println("MyfareVersion");

  unsigned char *buffer = (unsigned char *)bfr;
  unsigned char version[] = { 0, 0 };

  int len = ValidatorProtocolBuild(version, buffer, 2, PDT_STATUS_MIFARE_OK);

  ValidatorSend(buffer, len);
}

void CommandNotImplemented(void *bfr) {
  //serialRpi->println("CommandNotImplemented");

  unsigned char *buffer = (unsigned char *)bfr;

  int len = ValidatorProtocolBuild({}, buffer, 0, PDT_STATUS_MIFARE_INVALID_CMD);

  ValidatorSend(buffer, len);
}

//////////////////////////////////////////////
void validatorRfidSwitch(void *bfr) {
  unsigned char *buffer = (unsigned char *)bfr;

  switch (PRODATA_CMD(buffer)) {
    case PDT_MIFARE_REQUEST: MifareRequest(buffer); break;

    case PDT_MIFARE_ANTICOLLISION: MifareAnticolision(buffer); break;

    case PDT_MIFARE_SELECT: MifareSelect(buffer); break;

    case PDT_MIFARE_LOAD_KEY: MifareLoadKey(buffer); break;

    case PDT_MIFARE_AUTHENTICATION: MifareAuthentication(buffer); break;

    case PDT_MIFARE_READ: MifareRead(buffer); break;

    case PDT_MIFARE_WRITE: MifareWrite(buffer); break;

    case PDT_MIFARE_HALT: MifareHalt(buffer); break;

    case PDT_MIFARE_DECREMENT: MifareDecrement(buffer); break;

    case PDT_MIFARE_INCREMENT: MifareIncrement(buffer); break;

    case PDT_MIFARE_TRANSFER: MifareTransfer(buffer); break;

    case PDT_MIFARE_RESTORE: MifareRestore(buffer); break;

    case PDT_MIFARE_READ_SECTOR: MifareReadSector(buffer); break;

    case PDT_MIFARE_WRITE_SECTOR: MifareWriteSector(buffer); break;

    case PDT_MIFARE_RESET_RF: MifareResetRF(buffer); break;

    case PDT_MIFARE_POWER_DOWN: MifarePowerDown(buffer); break;

    case PDT_DISPLAY_WRITE_LINE:
    case PDT_DISPLAY_CLEAR:
    case PDT_CMD_BUZZER:
    case PDT_CMD_BUZZER_TMP:
    case PDT_CMD_KEEP_ALIVE:
      MifareNotImplemented(buffer);
      break;

    case PDT_CMD_REBOOT:
      // faz nada
      break;

    case PDT_CMD_GET_SERIAL:
      MifareSerial(buffer);
      break;
    case PDT_CMD_GET_VERSION:
      MifareVersion(buffer);
      break;

    default:
      CommandNotImplemented(buffer);
      break;
  }
}
