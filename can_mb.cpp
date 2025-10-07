#include <Arduino.h>

#include "can_mb.h"
#include "can.h"

// Deslocamento de bit
#define MBReadbit(value, bit) (((value) >> (bit)) & 0x01)

unsigned long canMBTotalTime = 0;
unsigned long canMBTotalPackets = 0;

bool canMBPacketReceived = false;

CANValue *canMBPointer;

// 17 = primeira
// 15 = neutro
// 14 = re
void CanMBParseGear(unsigned char *data)
{
  int tempData = 0;
  tempData = ((int)((data[4]) >> 2));

  if (tempData != 0xFF)
  {
    if (tempData >= 17 && tempData <= 31)
    {
      // Marchas adiante
      canMBPointer->gear = tempData - 16;
    }
    else if (tempData >= 0 && tempData <= 14.0)
    {
      // Ré
      canMBPointer->gear = -1;
    }
    else if (tempData == 15)
    {
      // Neutro
      canMBPointer->gear = 0;
    }
    else
    {
      // Valor inválido
      canMBPointer->gear = -15;
    }
  }
  // Serial.print("Gear: ");
  // Serial.println(canMBPointer->gear, DEC);
}

// Não está correto
// TO DO Testar
void CanMBParseHandbrake(unsigned char *data)
{
  if (data[3] <= 0xFA)
    canMBPointer->handbrake = (MBReadbit(data[3], 5) << 1) + MBReadbit(data[3], 4);

  // Serial.print("Handbrake: ");
  // Serial.println(canMBPointer->handbrake);
}

// OK
void CanMBParseFootbrake(unsigned char *data)
{
  if (data[3] <= 0xFA)
    canMBPointer->footbrake = (MBReadbit(data[3], 7) << 1) + MBReadbit(data[3], 6);

  // Serial.print("Footbrake: ");
  // Serial.println(canMBPointer->footbrake);
}

// Nao foi testado
void CanMBParseVehicleSpeed(unsigned char *data)
{
  float factor = 0.005;
  unsigned int tempData = 0;

  tempData = ((data[5] << 8) + data[4]) * factor;

  if (tempData < 120)
    canMBPointer->vehicleSpeed = tempData;

  // Serial.print("Speed: ");
  // Serial.println(canMBPointer->vehicleSpeed);
}

// OK
void CanMBParseEngineSpeed(unsigned char *data)
{
  unsigned long tempData = 0;

  tempData = data[7];
  tempData <<= 8;
  tempData |= data[6];

  tempData *= 4;
  tempData /= 25;

  if (tempData < 10000)
    canMBPointer->engineSpeed = tempData;

  /*
  canMBPointer->engineSpeed = data[7];
  canMBPointer->engineSpeed <<= 8;
  canMBPointer->engineSpeed |= data[6];

  canMBPointer->engineSpeed *= 4;
  canMBPointer->engineSpeed /= 25;

  if (canMBPointer->engineSpeed > 10000)
    canMBPointer->engineSpeed = 0;
*/

  // Serial.print("Engine Speed [rpm]: ");
  // Serial.println(canMBPointer->engineSpeed);
}

// Nao foi testado
void CanMBParseRetarder(unsigned char *data)
{
  // MsgID:304;Pos:18;Len:2;Factor:1
  unsigned char factor = 1;

  if (data[2] <= 0xFA)
    canMBPointer->retarder = ((data[2] & 0x0C) >> 2) + factor;

  // Serial.print("Retarder: ");
  // Serial.println(canMBPointer->retarder);
}

// OK
void CanMBParsePedalPosition(unsigned char *data)
{
  // MsgID:450;Pos:48;Len:8;Factor:0,4
  // float factor = 0.4;

  if (data[6] <= 0xFA)
  {
    canMBPointer->pedalPosition = data[6];
    canMBPointer->pedalPosition *= 2;
    canMBPointer->pedalPosition /= 5;
  }

  // Serial.print("Gaspedalpos: ");
  // Serial.println(canMBPointer->pedalPosition);
}

// OK
void CanMBParseAmbientTemperature(unsigned char *data)
{
  // MsgID:550;Pos:56;Len:8;Factor:0,5;Offset:-50
  int Offset = -50;
  float factor = 0.5;
  unsigned long tempData = 0;

  tempData = (data[7] * factor) + Offset;
  if (tempData < 70)
    canMBPointer->ambientTemperature = tempData;

  // canMBPointer->ambientTemperature = (data[7] * factor) + Offset;

  // Serial.print("Envtemp: ");
  // Serial.println(canMBPointer->ambientTemperature);
}

// OK
void CanMBParseEngineTemperature(unsigned char *data)
{
  // WaterTemp,MsgID:554;Pos:8;Len:8;Factor:1;Offset:-50
  // EngOilPress,MsgID:554;Pos:32;Len:8;Factor:0,04;Offset:0

  int offset = -50;
  unsigned long tempData = 0;

  tempData = data[1] + offset;

  if (tempData < 120)
    canMBPointer->engineTemperature = tempData;
  // canMBPointer->engineTemperature = data[1] + Offset;

  // Serial.print("WaterTemp: ");
  // Serial.println(canMBPointer->engineTemperature);
}

// Não tem como saber
void CanMBParseOilPressure(unsigned char *data)
{
  int offset = 1;
  // 2021 05 06
  float factor = 40.0;

  if (data[4] <= 0xFA)
    canMBPointer->oilPressure = (data[4] * factor) + offset;

  // Serial.print("Oil Pressure: ");
  // Serial.println(canMBPointer->oilPressure);
}

// OK
void CanMBParseAirSupplyPressure(unsigned char *data)
{
  // MsgID:5A0;Pos:40;Len:8;Factor:0,08;Offset:0
  // float factor = 0.08;
  // 2021 05 06
  float factor = 80.0;

  if (data[5] <= 0xFA)
    canMBPointer->airSupplyPressure = (data[5] * factor);

  // Serial.print("Air Pressure: ");
  // Serial.println(canMBPointer->airSupplyPressure);
}

// OK
void CanMBParseAlternator(unsigned char *data)
{
  // MsgID:5A0;Pos:56;Len:8;Factor:0,2;Offset:0
  float factor = 0.2;

  if (data[7] <= 0xFA)
    canMBPointer->alternator = (data[7] * factor);

  // Serial.print("Alternator: ");
  // Serial.println(canMBPointer->alternator);
}

// OK
void CanMBParseFuelLevel(unsigned char *data)
{
  // MsgID:6A0;Pos:40;Len:8;Factor:0,4;Offset:0
  float factor = 0.4;

  if (data[5] <= 0xFA)
    canMBPointer->fuelLevel = (data[5] * factor);

  // Serial.print("Fuel Level: ");
  // Serial.println(canMBPointer->fuelLevel);
}

// ToDo: Duvida no formato dos dados
void CanMBParseHighResDistance(unsigned char *data)
{
  // MsgID:6B5;Pos:0;Len:32;Factor:5;Offset:0
  float factor = 200;

  // canMBPointer->highResDistance = ((data[3] << 24) + (data[2] << 16) + (data[1] << 8) + (data[0])) / factor;

  unsigned long tempData = 0;

  for (char c = 3; c >= 0; c--)
  {
    tempData <<= 8;
    tempData |= data[(byte)c];
  }

  tempData /= factor;

  if (tempData < 10000000)
    canMBPointer->highResDistance = tempData;

  /*
  canMBPointer->highResDistance = 0;

  for (char c = 3; c >= 0; c--) {
    canMBPointer->highResDistance <<= 8;
    canMBPointer->highResDistance |= data[(byte)c];
  }

  canMBPointer->highResDistance /= factor;
  */
  // Serial.print("Hodometer: ");
  // Serial.println(canMBPointer->highResDistance);
}

// 4454 ml / 100 km - parece estar errado
void CanMBParseFuelEconomy(unsigned char *data)
{
  // MsgID:65E;Pos:0;Len:16;Factor:4;Offset:0
  float factor = 4;
  unsigned long tempData = 0;
  // canMBPointer->fuelEconomy = ((data[1] << 8) + (data[0])) * factor;

  tempData = data[1];
  tempData <<= 8;
  tempData |= data[0];

  tempData *= factor;

  if (tempData <= 257020)
    canMBPointer->fuelEconomy = (long)100000000 / tempData;

  // Serial.print("Fuel Economy: ");
  // Serial.println(canMBPointer->fuelEconomy);
}

// 3368 - verificar
void CanMBParseAdblueEconomy(unsigned char *data)
{
  // MsgID:65E;Pos:16;Len:16;Factor:0,4;Offset:0
  float factor = 0.4;
  unsigned long tempData = 0;

  tempData = data[3];
  tempData <<= 8;
  tempData |= data[2];

  tempData *= factor;

  if (tempData <= 257002)
    canMBPointer->adblueconsavg = tempData;

  // Serial.print("Adblueconsavg: ");
  // Serial.println(canMBPointer->adblueconsavg);
}

void CanMBParseAdblueLevel(unsigned char *data)
{
  // MsgID:65F;Pos:8;Len:8;Factor:0,4;Offset:0
  float factor = 0.4;
  unsigned long tempData = 0;

  tempData = (data[1] * factor);

  if (tempData <= 100)
    canMBPointer->adbluevol = tempData;

  // Serial.print("Adbluevol: ");
  // Serial.println(canMBPointer->adbluevol);
}

void CanMBDataParse(unsigned long canId, unsigned char *data, Stream *serial, struct CANValue &pointer)
{
  // unsigned long start = micros();
  unsigned long pgn = canId & 0x00FFFFFF;

  canMBPointer = &pointer;

  switch (pgn)
  {
  case CAN_MB_PACKET_GEAR:
    CanMBParseGear(data);
    canMBPacketReceived = true;
    break;

  case CAN_MB_PACKET_ENGINE_SPEED:
    CanMBParseHandbrake(data);
    CanMBParseFootbrake(data);
    CanMBParseVehicleSpeed(data);
    CanMBParseEngineSpeed(data);

    canMBPacketReceived = true;
    break;

  case CAN_MB_PACKET_RETARDER:
    CanMBParseRetarder(data);
    canMBPacketReceived = true;
    break;

  case CAN_MB_PACKET_PEDAL_POSITION:
    CanMBParsePedalPosition(data);
    canMBPacketReceived = true;
    break;

  case CAN_MB_PACKET_AMBIENT_TEMPERATURE:
    CanMBParseAmbientTemperature(data);
    canMBPacketReceived = true;
    break;

  case CAN_MB_PACKET_ENGINE_TEMPERATURE:
    CanMBParseEngineTemperature(data);
    CanMBParseOilPressure(data);
    canMBPacketReceived = true;
    break;

  case CAN_MB_PACKET_AIR_SUPPLY_PRESSURE:
    CanMBParseAirSupplyPressure(data);
    CanMBParseAlternator(data);
    canMBPacketReceived = true;
    break;

  case CAN_MB_PACKET_FUEL_LEVEL:
    CanMBParseFuelLevel(data);
    canMBPacketReceived = true;
    break;

  case CAN_MB_PACKET_HIGHRES_DISTANCE:
    /*
              serial->print("DATA: ");

              for (byte c = 0; c < 8; c++)
              serial->print (data [c], HEX);

              serial->println(".");
          */
    CanMBParseHighResDistance(data);
    /*
              serial->print("Hodometer: ");
              serial->println(canMBPointer->highResDistance, DEC);
              delay(1000);
          */
    canMBPacketReceived = true;
    break;

  case CAN_MB_PACKET_FUEL_CONSUMPTION:
    /*
              serial->print("DATA: ");

              for (byte c = 0; c < 8; c++)
              serial->print (data [c], HEX);

              serial->println(".");
              delay(1000);
          */
    CanMBParseFuelEconomy(data);
    CanMBParseAdblueEconomy(data);
    /*
              serial->print("Fuel Economy: ");
              serial->println(canMBPointer->fuelEconomy, DEC);

              delay(1000);
          */
    canMBPacketReceived = true;
    break;

  case CAN_MB_PACKET_ADBLUE_LEVEL:
    CanMBParseAdblueLevel(data);
    canMBPacketReceived = true;
    break;

  default:
    // serial->println("[MB] Packet not recognized");

    break;
  }

  // canMBTotalTime += micros() - start;

  if (canMBPacketReceived)
    canMBTotalPackets++;
}

unsigned long CanMBTotalPackets()
{
  return canMBTotalPackets;
}

unsigned long CanMBTotalTime()
{
  return canMBTotalTime;
}

bool CanMBPacketReceived()
{
  return canMBPacketReceived;
}

void CanMBResetPacketReceived()
{
  canMBPacketReceived = false;
}