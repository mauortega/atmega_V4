/*
    Compilação para NX7000
    - Arduino Pro / Pro Mini
    - Processor - ATMEGA328P - 5V - 16 MHz

    Observações:
    - Quando estiver como simulador CAN não colocar o CANLoop
    - Não colocar o watch dog e o simulador de CAN ao mesmo tempo

    2025/02/26 - Modo de SoftPowerOff para Raspberry
*/

#include <avr/wdt.h>
#include <SoftwareSerial.h>

#include "app.h"
#include "can.h"
#include "io.h"
#include "rfid.h"
#include "timer.h"
#include "protocol.h"
#include "validator.h"
#include "canSimulator.h"
#include "rpiWdt.h"
#include "config.h"

#include "tests.h"

// Conectado ao RX/TX da Raspi
#ifdef OPTION_SOFT_SERIAL
#define SW_SERIAL_RX_PIN 4
#define SW_SERIAL_TX_PIN 5
SoftwareSerial serialRpi(SW_SERIAL_RX_PIN, SW_SERIAL_TX_PIN);
#else
#define serialRpi Serial
#endif

// ********************** Setup e Loop **********************
void setup()
{
  byte reason = MCUSR;
  MCUSR = 0;
 
  wdt_disable();

  serialRpi.begin(SERIAL_BAUD);
  ProtocolSerialInit(serialRpi);

  serialRpi.println(F("[RESET] Noxxon SAT"));
  
  if (reason & (1 << PORF))
    Serial.println(F("Power-on Reset"));
  if (reason & (1 << EXTRF))
    Serial.println(F("External Reset"));
  if (reason & (1 << BORF))
    Serial.println(F("Brown-out Reset"));
  if (reason & (1 << WDRF))
    Serial.println(F("Watchdog Reset"));

  IOSetup();

  CheckI2CAddresses(&serialRpi);

  ConfigLoad(&serialRpi);

#ifdef OPTION_VALIDATOR
  ValidatorSetup(serialRpi);
#endif

#ifdef OPTION_CAN_SIMULATOR
  CanSetup(false);
#else
  CanSetCanBitRate(configData.canBitRate);
  CanSetListenMode(configData.canListenOnlyMode);
  CanSetup();
#endif

  RFIDSetup(&serialRpi);

  StartTimers();

  wdt_enable(WDTO_4S);
  wdt_reset();

  RpiWdtInit(&serialRpi);

#ifndef NX7000
  // ValidatorTestPacket(&serial);
#endif
}

void loop()
{
  wdt_reset();

  VerifyTimers();

#ifndef OPTION_CAN_SIMULATOR
  CanLoop(&serialRpi);
  ProtocolLoop();
#else
  CanSimulatorLoop(&serial);
#endif

ValidatorLoop();

#ifdef OPTION_CAN_EMULATOR
  unsigned int simulatorInput = ((unsigned long)analogRead(0) * 100) / 750;

  serialRpi.print("[SIM] Input: ");
  serialRpi.println(simulatorInput, DEC);

  CanEmulator(simulatorInput);

  serialRpi.print(F("[CAN] DATA:"));
  ProtocoloPrintHex(&serial, CanGetData(&serial), CanGetDataLen());

  delay(1000);
#endif
}

// ********************** Other functions **********************
void SendResume()
{
#ifdef OPTION_SEND_RESUME

  IOLoop(&serialRpi);

  IOStruct ioData = IOGetData();

  if (configData.serialProtocolLegacy) {
    serialRpi.print("[IOV2] ");
    ProtocoloPrintHex(&serialRpi, (unsigned char*)&ioData, sizeof(ioData));
  } else {
    SerialProtocolPrintHex(&serialRpi, (unsigned char *)&ioData, sizeof(ioData), FrameTypeIO);
  }
  
  if (CanDataReceived())
  {
    unsigned char *dataPtr = CanGetData(&serialRpi);

    if (configData.serialProtocolLegacy) {
      serialRpi.print(F("[CAN] DATA:"));
      ProtocoloPrintHex(&serialRpi, dataPtr, CanGetDataLen());
    } else {
      SerialProtocolPrintHex(&serialRpi, dataPtr, CanGetDataLen(), FrameTypeCAN);
    }
  }

  if (GetShutdownState() == RaspberryShutdownState)
  {
    serialRpi.println(F("[RASP] MainShutDown"));
  }

  /*
    if(CanStatus(& serial))
    {
        serial.println("[CAN_ERRROR] Error detected");
        CanConfigure();
    }
*/

  // serial.print("[WDT] TotalTime: ");
  // serial.println(WatchDogTotalTime(), DEC);

  // serial.print("[TEMP] Temperature: ");
  // serial.println(GetTemperature(), DEC);
#endif

#ifdef OPTION_RASP_WATCH_DOG
  RpiWdtLoop();
#endif

#ifdef OPTION_TEST_PROTOCOL
  // ProtocolTest(&serial);
#endif
}

void SimulatorLoop()
{
#ifdef VALIDATOR_SIMULATOR
  serialRpi.println("[VAL] - Simulator");
  ValidatorSimulator();
#endif
}

// ********************** Timers **********************

#define TIMERS_COUNT 3

Timer timers[TIMERS_COUNT] = {
    {0, 500, RfidLoop},
    {0, 1000, SendResume},
    {0, 10000, ProtocolSendVersion}
    //{ 0, 3000, SimulatorLoop }
};

void StartTimers()
{
  for (byte count = 0; count < TIMERS_COUNT; count++)
    timers[count].Start();
}

void VerifyTimers()
{
  for (byte count = 0; count < TIMERS_COUNT; count++)
    timers[count].Verify();
}
