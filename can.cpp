/************************************************************

NOTES:

2023/01/23
mcp_can.cpp
- Check if your version has function millis() assigned to long type

*/

#include "can.h"

#include "app.h"

#ifdef CAN_FMS
#include "can_fms.h"
#endif

#include "can_vw.h"
#include "can_mb.h"

// #define OPTION_CAN_EMULATOR_COUNTER
#define OPTION_CAN_EMULATOR_EVENTS

MCP_CAN CAN0(CAN_CS_PIN);

CANValue canValue;

static byte canBitRate = CAN_250K;
static bool canStarted = false;
static bool listenMode = false;

static Stream *serialRpi;

unsigned long CanPacketId;
unsigned char CanPacketLen = 0;
unsigned char CanDataBuffer[8];

bool CanPacketError = false;

int canVehicleLastSpeed = 0;

bool canDebugEnabled = false;

/// @brief Initializes the CAN bus
/// @return 
bool CanSetup()
{
  byte canSpeedConfig = CAN_250KBPS;

  switch (canBitRate)
  {
  case CAN_125K:
    canSpeedConfig = CAN_125KBPS;
    break;
  case CAN_250K:
    canSpeedConfig = CAN_250KBPS;
    break;
  case CAN_500K:
    canSpeedConfig = CAN_500KBPS;
    break;
  case CAN_1000K:
    canSpeedConfig = CAN_1000KBPS;
    break;
  }

  byte canResult = CAN0.begin(MCP_ANY, canSpeedConfig, CAN_CRYSTAL);

  if (canResult == CAN_OK)
    canStarted = true;
  else 
    canStarted = false;

  // Set operation mode to normal so the MCP2515 sends acks to received data.
  if (listenMode)
  {
    CAN0.setMode(MCP_LISTENONLY);
  }
  else
  {
    CAN0.setMode(MCP_NORMAL);
  }

  pinMode(CAN0_INT, INPUT);
}

/*****************************************************************************/
void CanSetCanBitRate(byte b)
{
  canBitRate = b;
}

byte CanGetCanBitRate()
{
  return canBitRate;
}

bool CanGetCanStarted()
{
  return canStarted;
}

void CanSetListenMode(byte b)
{
  listenMode = b;
}

void CanChangeNormalMode()
{
  CAN0.setMode(MCP_NORMAL);
}

void CanChangeListenMode()
{
  CAN0.setMode(MCP_LISTENONLY);
}

void CanLoop(Stream *serial)
{
  // If CAN0_INT pin is low, read receive buffer
  if (digitalRead(CAN0_INT) == true)
  {
    // Serial.println("[CAN] No data received");
    return;
  }

  digitalWrite(CAN_CS_PIN, HIGH);

  // Read data: len = data length, buf = data byte(s)
  CAN0.readMsgBuf(&CanPacketId, &CanPacketLen, CanDataBuffer);

  if (CanPacketLen == 8)
  {
    CanMBDataParse(CanPacketId, &CanDataBuffer[0], serial, canValue);
    CanVWDataParse(CanPacketId, &CanDataBuffer[0], serial, canValue);
#ifdef CAN_FMS
    CanFMSDataParse(CanPacketId, &CanDataBuffer[0], serial, canValue);
#endif
  }

  if (CanPacketLen == 0)
    CanPacketError = true;

#ifdef CAN_DEBUG
  if (canDebugEnabled)
  {
    char msgString[32];

    // Determine if ID is standard (11 bits) or extended (29 bits)
    if ((CanPacketId & 0x80000000) == 0x80000000)
      sprintf(msgString, "EID: %.8lX DLC: %1d", (CanPacketId & 0x1FFFFFFF), CanPacketLen);
    else
      sprintf(msgString, "SID: %.3lX DLC: %1d", CanPacketId, CanPacketLen);

    serial->print(msgString);

    for (byte i = 0; i < CanPacketLen; i++)
    {
      sprintf(msgString, " %.2X", CanDataBuffer[i]);
      serial->print(msgString);
    }

    serial->println();
  }
  // Para não travar a conexão serial
  // delay(1000);
#endif

  digitalWrite(CAN_CS_PIN, LOW);
}

void CanChangeSpeed(byte newSpeed, Stream *serialRpi)
{
  if (newSpeed != canBitRate)
  {
    canBitRate = newSpeed;
#ifndef OPTION_DISABLE_CAN
#ifndef OPTION_CAN_SIMULATOR
    CanSetup();
#endif
#endif
  }
}

bool CanSimulator()
{
  byte dataSimulated[8] = {0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00};

  byte sendStatus = CAN0.sendMsgBuf(0x88FEF100, 0, 8, dataSimulated);

  if (sendStatus == CAN_OK)
    return true;
  else
    return false;
}

bool CanSimulatorSend(Stream *serial, unsigned long id, byte *data)
{
  bool extendedId = (id > 0xffff ? true : false);

  byte sendStatus = CAN0.sendMsgBuf(id, extendedId, 8, data);

  CanMBDataParse(id, data, serial, canValue);
  CanVWDataParse(id, data, serial, canValue);

#ifdef CAN_DEBUG
  if (CanMBPacketReceived() || CanVWPacketReceived())
  {
    // MB
    // serial->print(" Engine Speed [rpm]: ");
    // serial->print(canValue.engineSpeed, DEC);

    // MB
    // serial->print(" Pedal: ");
    // serial->print(canValue.pedalPosition, DEC);

    // serial->print("Velocidade [Km/h]: ");
    // serial->print (canValue.vehicleSpeed, DEC);

    // serial->print(" Gear: ");
    // serial->print(canValue.gear, DEC);

    // serial->print(" Consumo Medio [Km/mg]: ");
    // serial->print(canValue.fuelEconomy, DEC);

    // serial->print(" Doors: ");
    // serial->print(canValue.doorsControl1, DEC);

    // serial->print(" POleo [mbar]: ");
    // serial->print(canValue.oilPressure, DEC);

    // serial->print(" PComb [mbar]: ");
    // serial->print(canValue.fuelPressure, DEC);

    // serial->print(" PBrake: ");
    // serial->print(canValue.parkingBreak, DEC);

    // serial->print(" BSwitch: ");
    // serial->print(canValue.breakSwitch, DEC);

    // serial->print(" TAmbiente [C]: ");
    // serial->print(canValue.ambientTemperature, DEC);

    // serial->print(" Bat [V]: ");
    // serial->print(canValue.alternator, DEC);

    // serial->print(" EHours [h]: ");
    // serial->print(canValue.engineHours, DEC);

    // serial->print(" Temp Motor [C]: ");
    // serial->print(canValue.engineTemperature, DEC);

    // serial->print(" AirPres [mbar]: ");
    // serial->print(canValue.airSupplyPressure, DEC);

    // serial->print(" CTotal [l]: ");
    // serial->print(canValue.fuelConsumption, DEC);

    // serial->print(" Hodo [Km]: ");
    // serial->print(canValue.highResDistance, DEC);

    // serial->print(" ACombustivel: ");
    // serial->print(canValue.waterInFuel, DEC);

    serial->println("");
  }
#endif

  if (sendStatus == CAN_OK)
    return true;
  else
    return false;
}

unsigned long canLastPacketReceived = 0;

unsigned char *CanGetData(Stream *serial)
{
  unsigned char *result = (unsigned char *)&canValue;

  canValue.protocolVersion = CAN_VW_VER2;

  if (CanMBPacketReceived())
    canValue.protocolVersion = CAN_MB_VER2;

  if (CanVWPacketReceived())
    canValue.protocolVersion = CAN_VW_VER2;

  canValue.vehicleSpeedDiff = canValue.vehicleSpeed - canVehicleLastSpeed;

  // CPQD
  // if (canValue.vehicleSpeed == canVehicleLastSpeed)
  //   CanSetup(CAN_250K, MCP_16MHZ, false);  //CPQD

  canVehicleLastSpeed = canValue.vehicleSpeed;

  CanMBResetPacketReceived();
  CanVWResetPacketReceived();

#ifdef CAN_FMS
  CanFMSResetPacketReceived();
#endif

  if (canDebugEnabled)
  {
    serial->print(F("[ENGINE_SPEED] "));
    serial->println(canValue.engineSpeed, DEC);
  }

  canLastPacketReceived = millis();

  return result;
}

unsigned int CanGetDataLen()
{
  // ToDo: Corrigir o tamanho
  return sizeof(CANValue);
}

bool CanDataReceived()
{
  bool result = false;

  if (CanMBPacketReceived())
    result = true;

  if (CanVWPacketReceived())
    result = true;

#ifdef CAN_FMS
  if (CanFMSPacketReceived())
    result = true;
#endif

  return result;
}

void CanEmulator(unsigned int value)
{
  // memset((void *)&canValue, 0, sizeof(canValue));

  // canValue.protocolVersion = CAN_MB_VER2;

  int counter = 1;

#ifdef OPTION_CAN_EMULATOR_COUNTER

  canValue.timeStamp = counter++;
  canValue.engineSpeed = counter++;
  canValue.fuelConsumption = counter++;
  canValue.fuelLevel = counter++;
  canValue.engineHours = counter++;
  canValue.tachograph = counter++;
  canValue.highResDistance = counter++;
  canValue.engineTemperature = counter++;
  canValue.ambientTemperature = counter++;
  canValue.airSupplyPressure = counter++;
  canValue.fuelEconomy = counter++;
  canValue.vehicleSpeed = counter++;
  canValue.vehicleSpeedDiff = counter++;
  canValue.pedalPosition = counter++;
  canValue.doorsControl1 = counter++;
  canValue.doorsControl2 = counter++;
  canValue.alternator = counter++;
  canValue.gear = counter++;
  canValue.oilPressure = counter++;
  canValue.fuelPressure = counter++;
  canValue.parkingBreak = counter++;
  canValue.breakSwitch = counter++;
  canValue.waterInFuel = counter++;
  canValue.handbrake = counter++;
  canValue.footbrake = counter++;
  canValue.retarder = counter++;
  canValue.adblueconsavg = counter++;
  canValue.adbluevol = counter++;
#endif

#ifdef OPTION_CAN_EMULATOR_EVENTS

  switch (value / 5)
  {
  case 0:
    break;
  case 1:
    canValue.pedalPosition = 25;
    canValue.gear = 0;
    break;
  case 2:
    canValue.engineSpeed = 3000;
    break;
  case 3:
    canValue.oilPressure = 6000;
    break;
  case 4:
    canValue.oilPressure = 1000;
    break;
  case 5:
    canValue.alternator = 19;
    break;
  case 6:
    canValue.engineSpeed = 80;
    break;
  case 7:
    canValue.ambientTemperature = 45;
    break;
  case 8:
    canValue.engineTemperature = 103;
    break;
  case 9:
    canValue.retarder = 1;
    break;
  case 10:
    canValue.adbluevol = 15;
    break;
  case 11:
    canValue.vehicleSpeedDiff = -15;
    break;
  case 12:
    canValue.vehicleSpeedDiff = 15;
    break;
  case 13:
    canValue.vehicleSpeed = 70;
    break;
  case 14:
    canValue.vehicleSpeed = 12;
    canValue.gear = 0;
    break;
  }
#endif
}

bool CanStatus(Stream *serial)
{
  byte error = CAN0.checkError();

  if (error != CAN_OK)
  {
    byte error = CAN0.getError();

    serial->print(F("[CAN] Error detected: "));
    serial->println(error, HEX);

    return true;
  }

  return false;
}

void CanDebug(const char *format, unsigned long value)
{
#ifdef CAN_DEBUG
  char debugBuffer[50];

  sprintf(debugBuffer, format, value);

  Serial.println(debugBuffer);
#endif
}

void CanDebug(const __FlashStringHelper *format, unsigned long value)
{
#ifdef CAN_DEBUG
  char debugBuffer[50];

  sprintf(debugBuffer, (const char *)format, value);

  Serial.println(debugBuffer);
#endif
}

void CanDebug(Stream *serial, const __FlashStringHelper *format, unsigned long value)
{
#ifdef CAN_DEBUG
  char debugBuffer[50];

  sprintf(debugBuffer, (const char *)format, value);

  serial->println(debugBuffer);
#endif
}

void SetCanDebug(bool enabled)
{
  canDebugEnabled = enabled;
}