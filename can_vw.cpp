#include <Arduino.h>

#include "can_vw.h"
#include "can.h"

unsigned long canVWTotalTime = 0;
unsigned long canVWTotalPackets = 0;

bool canVWPacketReceived = false;

CANValue *canVWPointer;

// CAN_VW_PACKET_ENGINE_SPEED          0xF004
// Valor máximo J1939 FAFF = 8031.875
// Valores maiores que esse indicam problema no sensor ou indisponivilidade da informação
void CanVWParseEngineSpeed(unsigned char *data)
{
  unsigned int value = 0;

  value = ((data[4] << 8) + data[3]) / 8;

  if (value < 8031)
    canVWPointer->engineSpeed = value;
}

// CAN_VW_PACKET_PEDAL_POSITION			  0xF003
// Valor máximo J1939 FA = 250 -> 100%
void CanVWParsePedalPosition(unsigned char *data)
{
  const float factor = 0.4;
  unsigned int value = 0;

  value = (data[1] * factor);
  if (value <= 100)
    canVWPointer->pedalPosition = value;
}

// CAN_VW_PACKET_TACHOGRAPH				    0xFE6C
// Valor máximo J1939 FAFF = 250.996
void CanVWParseTachograph(unsigned char *data)
{
  const unsigned int factor = 256;
  unsigned long value = 0;

  value = data[7];
  value <<= 8;
  value |= data[6];

  value /= factor;

  if (value <= 125)
  {
    canVWPointer->tachograph = value;
    canVWPointer->vehicleSpeed = value;
  }
}

// CAN_VW_PACKET_GEAR						      0xF005
// 0 = 0
// 130 = 1
// re = -1
// Nao ok
void CanVWParseGear(unsigned char *data)
{
  const unsigned int factor = 125;
  char value = 0;

  value = data[3];
  if (value <= 252)
  {

    value = data[3] - factor;
    // Parking
    if (data[3] == 251)
      canVWPointer->gear = 0;
    else
      canVWPointer->gear = value;
  }
}

// CAN_VW_PACKET_VEHICLE_SPEED				  0xFEBF
// Teste executado em simulador
void CanVWParseVehicleSpeed(unsigned char *data)
{
  const unsigned int factor = 256;
  unsigned long value = 0;

  value = data[0];
  value <<= 8;
  value |= data[1];

  value /= factor;

  if (value <= 125)
  {
    canVWPointer->vehicleSpeed = value;
  }
}

// CAN_VW_PACKET_FUEL_ECONOMY				  0xFEF2
// Parece errado - 1970 (no veiculo mostra 1.9 Km/l)
// Valor que aparece no data[5] e data[4] precisa ser dividio por 512 ao multiplicar por 1000 temos metros/Litros
void CanVWParseFuelEconomy(unsigned char *data)
{
  const unsigned int factor = 512;
  unsigned long value = 0;

  value = data[3];
  value <<= 8;
  value |= data[2];

  value /= factor;

  if (value <= 125)
    canVWPointer->fuelEconomy = value * 1000; // porque o main divide por 1000 Valor é em km/L
  // canVWPointer->fuelEconomy = (((long)((data[3] << 8) + data[2])) * 1000) / 512;
}

enum DOORS_STATUS
{
  ALL_DISABLED = 0x00,
  AT_LEAST_ONE_ENABLED = 0x01,
  ERROR = 0x02,
  NOT_AVAILABLE = 0x03
};

enum DOORS_POSITION
{
  AT_LEAST_ONE_OPEN = 0x00,
  CLOSING_LAST_DOOR = 0x01,
  ALL_CLOSED = 0x02,
  // NOT_AVAILABLE = 0x03
};

// CAN_VW_PACKET_DOORS_CONTROL1			  0xFE4E
/*
0000 At least 1 door is open
0001 Closing last door
0010 All doors closed
0011-1101 Not defined
1110 Error
1111 Not available
*/
void CanVWParseDoorsControl1(unsigned char *data)
{

  unsigned long value = 0;
  value = data[0];
  if (value != 0xFF)
  {
    // canVWPointer->doorsControl1 = data[0] & 0x0F;

    switch ((value & 0x0F))
    {
    // Pelo menos uma porta aberta
    case 0:
      canVWPointer->doorsControl1 = 0;
      break;

    // Fechando ultima porta
    case 1:
      canVWPointer->doorsControl1 = 0;
      break;

    // Todas as portas fechadas
    case 0x02:
      canVWPointer->doorsControl1 = 1;
      break;

    // Pelo menos uma porta aberta
    default:
      canVWPointer->doorsControl1 = 0;
      break;
    }
  }
}

// CAN_VW_PACKET_OIL_PRESSURE				  0xFEEF
// Valor é dividido por 1000 no main. Valor em Pascoal não KPa
void CanVWParseOilPressure(unsigned char *data)
{
  // ToDo: CPQD - Colocar 4
  // unsigned int factor = 40;
  unsigned int factor = 4;
  unsigned int value = 0;

  value = ((unsigned int)data[3]) * factor;
  if (value <= 1000)
    canVWPointer->oilPressure = value;
}

// CAN_VW_PACKET_FUEL_PRESSURE				  0xFEEF
// Valor NÃO é dividido por 1000 no main. Valor em KPa
void CanVWParseFuelPressure(unsigned char *data)
{
  // unsigned int factor = 40;

  unsigned int factor = 4;
  unsigned int value = 0;

  value = ((unsigned int)data[0]) * factor;

  if (value <= 1000)
    canVWPointer->fuelPressure = value;
}

// CAN_VW_PACKET_PARKING_BRAKE				  0xFEF1
void CanVWParseParkingBrake(unsigned char *data)
{
  const unsigned int factor = 256;
  unsigned long value = 0;

  value = data[0];
  if (value != 0xFF)
  {
    value = (data[0] & 0x0C);
    value >>= 2;
    canVWPointer->parkingBreak = (value == 0x01 ? 1 : 0);
  }

  value = data[3];
  if (value != 0xFF)
  {
    value = (data[3] & 0xC0);
    value >>= 6;
    canVWPointer->clutchSwitch = (value == 0x01 ? 1 : 0);

    value = (data[3] & 0x30);
    value >>= 4;
    canVWPointer->breakSwitch = (value == 0x01 ? 1 : 0);
  }
}

// CAN_VW_PACKET_AMBIENT_TEMPERATURE		0xFEF5
void CanVWParseAmbientTemperature(unsigned char *data)
{
  int factor = -273;
  unsigned int constant = 32;
  long value = 0;

  value = (((data[4] << 8) + data[3]) / constant) + factor;
  if (value <= 200)
    canVWPointer->ambientTemperature = value;

  value = (((data[2] << 8) + data[1]) / constant) + factor;
  if (value <= 200)
    canVWPointer->cabinTemperature = value;
}

// CAN_VW_PACKET_ALTERNATOR				    0xFEF7
void CanVWParseAlternator(unsigned char *data)
{
  float factor = 20;
  unsigned long value = 0;

  // Byte 2 - corrente do alternador
  // Byte 4-5 - Tensao do alternador
  value = ((data[5] << 8) + data[4]) / 20;
  if (value < 100)
    canVWPointer->alternator = value;
}

// CAN_VW_PACKET_ENGINE_HOURS				  0xFEE5
void CanVWParseEngineHours(unsigned char *data)
{
  unsigned int factor = 20;
  unsigned long value = 0;

  for (char c = 3; c >= 0; c--)
  {
    value <<= 8;
    value |= data[(byte)c];
  }

  value /= factor;

  if (value <= 210000000)
    canVWPointer->engineHours = value;
}

// CAN_VW_PACKET_ENGINE_TEMPERATURE		0xFEEE
//-40 to 210 deg C
void CanVWParseEngineTemperature(unsigned char *data)
{
  int factor = -40;
  unsigned long value = 0;

  value = data[0] + factor;

  if (value <= 210)
    canVWPointer->engineTemperature = value;
}

// CAN_VW_PACKET_AIR_SUPPLY_PRESSURE		0xFEAE
// Main divide por 1000 está em Pascal
// j1939 diz fator de 8
// 0 to 2000 KPa
void CanVWParseAirSupplyPressure(unsigned char *data)
{
  unsigned int factor = 8;
  unsigned long value = 0;

  value = data[3] * factor;
  if (value <= 2000)
    canVWPointer->airSupplyPressure = value;
}

// CAN_VW_PACKET_FUEL_CONSUMPTION			0xFEE9
// 0.5L/bit TOTAL desde que o motor foi ligado nesse controlador
//  0 to 2,105,540,607.5L = 0xFAFFFFFF / 2
void CanVWParseFuelConsumption(unsigned char *data)
{

  char factor = 2;
  unsigned long value = 0;

  for (char c = 7; c >= 4; c--)
  {
    value <<= 8;
    value |= data[(byte)c];
  }

  value /= factor;
  if (value <= 2105540607)
    canVWPointer->fuelConsumption = value;
}

// CAN_VW_PACKET_HIGHRES_DISTANCE			0xFEC1
// Valor é 5m/bit por isso o valor de 5/1000 = 0.005
// 0 to 21055406 km
void CanVWParseHighResDistance(unsigned char *data)
{
  float factor = 0.005;

  unsigned long value = 0;

  for (char c = 3; c >= 0; c--)
  {
    value <<= 8;
    value |= data[(byte)c];
  }

  value *= factor;

  if (value < 20000000)
  {
    if (canVWPointer->highResDistance == 0)
    {
      canVWPointer->highResDistance = value;
    }
    else if ((value >= canVWPointer->highResDistance) && ((value - canVWPointer->highResDistance) < 100))
    {
      canVWPointer->highResDistance = value;
    }
  }
}

// Teste executado em simulador
/*
    void CanVWParseDateTime(unsigned char * data)
    {
  canVWPointer->dateTime = 0;

  for (char c = 3; c >= 0; c--)
  {
    canVWPointer->dateTime <<= 8;
    canVWPointer->dateTime |= data[c];
  }

  ////Serial.print("Data Hora: ");
  ////Serial.println(canVWPointer->dateTime);
    }
*/

// CAN_VW_PACKET_WATER_IN_FUEL				  0xFEFF
// 00 - No
// 01 - Yes
// 10 - Error
// 11 - Not available
void CanVWParseWaterInFuel(unsigned char *data)
{
  unsigned long value = 0;

  value = data[0];

  if (value != 0xFF)
    canVWPointer->waterInFuel = (value & 0x03);
}

// CAN_VW_PACKET_FUEL_LEVEL            0xFEFC
//  0.4 %/bit, 0 offset
//  0 to 100%
void CanVWParseFuelLevel(unsigned char *data)
{
  const float factor = 0.4;
  unsigned long value = 0;

  value = (data[1] * factor);

  if (value <= 100)
    canVWPointer->fuelLevel = value;
}

void CanVWParseTellTale(unsigned char *data)
{
  unsigned char blockId = data[0] & 0x0F;

  if (blockId == 0x00)
  {
#ifdef CAN_FMS
    canVWPointer->airConditioning = ((data[0] & 0x70) == 0x00 ? 0 : 1);
    canVWPointer->lights = ((data[1] & 0x70) == 0x10 ? 1 : 0);
#endif
  }
}

// Comando para receber dados de Telemetria da MIX
void CanMIXMCPNormal(unsigned char *data)
{
  CanChangeNormalMode();
}

// CAN_PACKET_HIGHRES_FUEL_CONSUMPTION 0xFD09
// 0.001L/bit TOTAL desde que o motor foi ligado nesse controlador
//  0 to 4,211,081.215L = 0xFAFFFFFF / 1000
// Main está dividindo por 1000
void CanVWParseHighResFuelConsumption(unsigned char *data)
{
  unsigned long value = 0;

  for (char c = 7; c >= 4; c--)
  {
    value <<= 8;
    value |= data[(byte)c];
  }

  if (value < 2000000000)
  {
    if (canVWPointer->fuelConsumption == 0)
      canVWPointer->fuelConsumption = value;

    else if ((value >= canVWPointer->fuelConsumption) && ((value - canVWPointer->fuelConsumption < 1000000)))
    {
      canVWPointer->fuelConsumption = value;
    }
  }
}

// CAN_VW_PACKET_ARLA_LEVEL       			0xFE56
//  0.4 %/bit, 0 offset
//  0 to 100%
void CanVWArlaLevel(unsigned char *data)
{
  const float factor = 0.4;
  unsigned long value = 0;

  value = (data[0] * factor);

  if (value <= 100)
    canVWPointer->adbluevol = value;
}

void CanVWDataParse(unsigned long canId, unsigned char *data, Stream *serial, struct CANValue &pointer)
{
  // unsigned long start = micros();

  unsigned long pgn = (canId & 0x00FFFF00) >> 8;

  canVWPointer = &pointer;

  switch (pgn)
  {
  case CAN_VW_PACKET_ENGINE_SPEED:
    CanVWParseEngineSpeed(data);
    canVWPacketReceived = true;

    // serial->print("Engine Speed: ");
    // serial->println(canVWPointer->engineSpeed, DEC);

    break;

  case CAN_VW_PACKET_PEDAL_POSITION:
    CanVWParsePedalPosition(data);
    canVWPacketReceived = true;

    // serial->print("Pedal Position: ");
    // serial->println(canVWPointer->pedalPosition, DEC);
    break;

  case CAN_VW_PACKET_TACHOGRAPH:
    CanVWParseTachograph(data);
    canVWPacketReceived = true;

    // serial->print("Tachograph: ");
    // serial->println(canVWPointer->tachograph, DEC);
    break;

  case CAN_VW_PACKET_GEAR:
    CanVWParseGear(data);
    canVWPacketReceived = true;

    // serial->print("Gear: ");
    // serial->println(canVWPointer->gear, DEC);
    break;

  case CAN_VW_PACKET_VEHICLE_SPEED:
    CanVWParseVehicleSpeed(data);
    canVWPacketReceived = true;

    // serial->print("VehicleSpeed: ");
    // serial->println(canVWPointer->vehicleSpeed, DEC);
    break;

  case CAN_VW_PACKET_FUEL_ECONOMY:
    CanVWParseFuelEconomy(data);
    canVWPacketReceived = true;

    // serial->print("Fuel Economy: ");
    // serial->println(canVWPointer->fuelEconomy, DEC);

    break;

  case CAN_VW_PACKET_DOORS_CONTROL1:
    CanVWParseDoorsControl1(data);
    canVWPacketReceived = true;
    break;

  case CAN_VW_PACKET_OIL_PRESSURE:
    CanVWParseOilPressure(data);
    CanVWParseFuelPressure(data);
    canVWPacketReceived = true;

    // serial->print("O pressure: ");
    // serial->println(canVWPointer->oilPressure, DEC);

    // serial->print("F pressure: ");
    // serial->println(canVWPointer->fuelPressure, DEC);
    break;

  case CAN_VW_PACKET_PARKING_BRAKE:
    if ((canId & 0x000000FF) == 0x00)
    {
      CanVWParseParkingBrake(data);
    }

    // serial->print("ParkingBrake: ");
    // serial->println(canVWPointer->parkingBreak, DEC);

    // serial->print("BreakSwitch: ");
    // serial->println(canVWPointer->breakSwitch, DEC);

    canVWPacketReceived = true;

    break;

  case CAN_VW_PACKET_AMBIENT_TEMPERATURE:
    CanVWParseAmbientTemperature(data);
    canVWPacketReceived = true;

    // serial->print("AmbientTemperature: ");
    // serial->println(canVWPointer->ambientTemperature, DEC);
    break;

  case CAN_VW_PACKET_ALTERNATOR:
    CanVWParseAlternator(data);
    canVWPacketReceived = true;

    // serial->print("Alternator: ");
    // serial->println(canVWPointer->alternator, DEC);
    break;

  case CAN_VW_PACKET_ENGINE_HOURS:
    CanVWParseEngineHours(data);
    canVWPacketReceived = true;

    // serial->print("EngineHours: ");
    // serial->println(canVWPointer->engineHours, DEC);
    break;

  case CAN_VW_PACKET_ENGINE_TEMPERATURE:
    CanVWParseEngineTemperature(data);
    canVWPacketReceived = true;

    // serial->print("EngineTemperature: ");
    // serial->println(canVWPointer->engineTemperature, DEC);
    break;

  case CAN_VW_PACKET_AIR_SUPPLY_PRESSURE:
    CanVWParseAirSupplyPressure(data);
    canVWPacketReceived = true;

    // serial->print("AirSupplyPressure: ");
    // serial->println(canVWPointer->airSupplyPressure, DEC);
    break;

  case CAN_VW_PACKET_FUEL_CONSUMPTION:
    CanVWParseFuelConsumption(data);
    canVWPacketReceived = true;

    // serial->print("Fuel Consumption: ");
    // serial->println(canVWPointer->fuelConsumption, DEC);

    break;

  case CAN_VW_PACKET_HIGHRES_DISTANCE:
    if ((canId & 0x000000FF) == 0x17)
    {
      CanVWParseHighResDistance(data);
      canVWPacketReceived = true;
    }
    // serial->print("HighResDistance: ");
    // serial->println(canVWPointer->highResDistance, DEC);
    break;

    /*
          case CAN_VW_PACKET_DATE_TIME:
          CanParseDateTime(data);
          break;
      */

  case CAN_VW_PACKET_WATER_IN_FUEL:
    CanVWParseWaterInFuel(data);
    canVWPacketReceived = true;
    break;

  case CAN_VW_PACKET_FUEL_LEVEL:
    CanVWParseFuelLevel(data);
    canVWPacketReceived = true;
    break;

  case CAN_VW_PACKET_AIR_CONDITIONING:
    CanVWParseTellTale(data);
    canVWPacketReceived = true;
    break;

  case CAN_MIX_MCP_NORMAL:
    CanMIXMCPNormal(data);
    canVWPacketReceived = true;
    break;

  case CAN_VW_PACKET_ARLA_LEVEL:
    CanVWArlaLevel(data);
    canVWPacketReceived = true;
    break;

  case CAN_PACKET_HIGHRES_FUEL_CONSUMPTION:
    CanVWParseHighResFuelConsumption(data);
    canVWPacketReceived = true;
    break;

  default:
    // serial->println("[VW] Packet not recognized");
    // canVWPacketReceived = false;
    break;
  }

  // canVWTotalTime += micros() - start;

  if (canVWPacketReceived)
    canVWTotalPackets++;
}

unsigned long CanVWTotalPackets()
{
  return canVWTotalPackets;
}

unsigned long CanVWTotalTime()
{
  return canVWTotalTime;
}

bool CanVWPacketReceived()
{
  return canVWPacketReceived;
}

void CanVWResetPacketReceived()
{
  canVWPacketReceived = false;
}
