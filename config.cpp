
#include <Arduino.h>
#include "config.h"
#include <EEPROM.h>
#include "can.h"

Configuracoes configData;

// Endereço inicial da EEPROM
const int EEPROM_ADDR = 0;

static Stream *serialRpi;
static bool configLoaded = false;

void ConfigSave()
{
    configData.assinatura = SIGNATURE;   // Assinatura para validar os dados
    EEPROM.put(EEPROM_ADDR, configData); // salva struct completa
    serialRpi->println(F("[CONFIG] EEPROM saved"));
    // ConfigLoad(serialRpi);
    // ConfigPrint();
}

void ConfigLoad(Stream *serial)
{
    serialRpi = serial;
    EEPROM.get(EEPROM_ADDR, configData); // carrega struct completa

    // Verifica se a assinatura bate
    if (configData.assinatura != SIGNATURE)
    {
        serialRpi->println(F("[CONFIG] EEPROM no data"));

        // Assinatura válida
        configData.assinatura = SIGNATURE;

        // Configurações padrão
        configData.canBitRate = CAN_250K;
        configData.validatorRfidEnabled = 0;
        configData.validatorRfidDirect = 0;
        configData.canListenOnlyMode = 0; 
        configData.serialProtocolLegacy = 1;

        EEPROM.put(EEPROM_ADDR, configData);
    }
    else
    {
        serialRpi->println(F("[CONFIG] EEPROM loaded"));
        configLoaded = true;
    }
}

void ConfigPrint()
{
    serialRpi->print(F("[CONFIG] loaded: "));
    serialRpi->println(configLoaded ? "YES" : "NO");
    serialRpi->print(F("[CONFIG] canBitRate: "));
    serialRpi->println(configData.canBitRate);
    serialRpi->print(F("[CONFIG] validatorRfidEnabled: "));
    serialRpi->println(configData.validatorRfidEnabled);
    serialRpi->print(F("[CONFIG] validatorRfidDirect: "));
    serialRpi->println(configData.validatorRfidDirect);
    serialRpi->print(F("[CONFIG] canListenOnlyMode: "));
    serialRpi->println(configData.canListenOnlyMode);
    serialRpi->print(F("[CONFIG] serialProtocolLegacy: "));
    serialRpi->println(configData.serialProtocolLegacy);
}
