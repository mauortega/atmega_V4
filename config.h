#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>

// Valor fixo para validar as configs salvas
const unsigned int SIGNATURE = 0xBEEF;  

// Struct de configuração
struct Configuracoes {
  unsigned int assinatura;  // primeiro campo = assinatura
  byte canBitRate;
  byte validatorRfidEnabled;
  byte validatorRfidDirect;
  byte canListenOnlyMode;
  byte serialProtocolLegacy; // 0 = frame protocol, 1 = legacy protocol
  byte reserved[5]; // Reservado para futuras configurações
};

extern Configuracoes configData; // Variável global para acessar as configurações

void ConfigSave();
void ConfigLoad(Stream *serial);
void ConfigPrint();

#endif