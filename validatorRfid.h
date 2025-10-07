#ifndef validator_RFID_h
#define validator_RFID_h

#include <Arduino.h>

void ValidatorRfidSetup(Stream &debug_port);


// calls the appropriate command
void validatorRfidSwitch(void *bfr);

// //	0x31	Mifare request – requisição de cartão em idle state, LEN = 0x01, DATA = 0x26 request idle state, DATA = 0x52 request all cards.
// void MifareRequest(void *bfr);

// //	0x32	Mifare anticolision – inicia processo de anticolisão
// void MifareAnticolision(void *bfr);

// //	0x33	Mifare select – seleciona cartão, LEN = 0x04, DATA = card serial number, DATA (byte 6) = MSB, DATA (byte 3) = LSB
// void MifareSelect(void *bfr);

// //	0x34	Mifare load key – carrega 6 bytes, chave de autenticação. LEN = 0x06, DATA (byte 3) = MSB, DATA (byte 8) = LSB
// void MifareLoadKey(void *bfr);

// //	0x35	Mifare authentication – comando de autenticação – LEN = 0x06, se DATA (byte 3) = 0x60 chave A, se DATA (byte 3) = 0x61 chave B
// void MifareAuthentication(void *bfr);

// //	0x36	Mifare read – comando de leitura – LEN = 0x01, DATA = block address
// void MifareRead(void *bfr);

// //	0x37	Mifare write – comando de escrita – LEN = 0x11, DATA (byte 3) = block address, DATA (byte 4 - 19) Dados a serem escritos, byte 4 MSB
// void MifareWrite(void *bfr);

// //	0x38	Mifare halt – comando para parar o processo
// void MifareHalt(void *bfr);

// //	0x39	Mifare decrement - comando de decremento – LEN = 0x05, DATA (byte 3) = block address (valor maximo de 63), converte DATA (byte 4 a 7) em um valor inteiro de 32 bits sendo byte 4 LSB e byte 7 MSB.
// void MifareDecrement(void *bfr);

// //	0x3A	Mifare increment – comando de incremento - LEN = 0x05, DATA (byte 3) = block address (valor maximo de 63), converte DATA (byte 4 a 7) em um valor inteiro de 32 bits sendo byte 4 LSB e byte 7 MSB.
// void MifareIncrement(void *bfr);

// //	0x3B	Mifare transfer – transfere valor – LEN = 0x01, DATA = block address (valor maximo de 63)
// void MifareTransfer(void *bfr);

// //	0x3C	Mifare restore – comando restaurar - LEN = 0x01, DATA = block address (valor maximo de 63)
// void MifareRestore(void *bfr);

// //	0x3D	Mifare read sector – lê setor - LEN = 0x01, DATA = sector address (valor maximo de 15) Primeiro bloco lido = DATA (byte 3) multiplicado por 4, 16 bytes – resposta DATA[3 ao 18] Segundo bloco lido = DATA (byte 3+1) multiplicado por 4, 16 bytes – resposta DATA[19 ao 34] Terceiro bloco lido = DATA (byte 3) multiplicado por 4, 16 bytes – resposta DATA[35 ao 50]
// void MifareReadSector(void *bfr);

// //	0x3E	Mifare write sector – escreve em setor - LEN = 0x31, DATA (byte 3) multiplicado por 4 = sector address do primeiro bloco (valor maximo de 15 do byte 3), DATA (byte 4) = MSB first block, DATA (byte 20) = MSB second block, DATA (byte 36) = MSB third block
// void MifareWriteSector(void *bfr);

// //	0x3F	Mifare reset RF – reseta o radio
// void MifareResetRF(void *bfr);

// //	0x40	Mifare power down – desliga o radio - LEN = 0x01, se DATA = 0x01 desliga, se DATA = 0x00 liga
// void MifarePowerDown(void *bfr);

// void MifareNotImplemented(void *bfr);

// void MifareSerial(void *bfr); 

// void MifareVersion(void *bfr);

// void CommandNotImplemented(void *bfr);


#endif
