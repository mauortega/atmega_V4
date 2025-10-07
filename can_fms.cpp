#include "can.h"
#include "can_fms.h"

#ifdef CAN_FMS

//CANValue fmsValue;

CANValue * canFMSPointer;

bool canFMSPacketReceived = false;

void CanFMSParseFuelConsumption(unsigned char *data)
{
    const unsigned int factor = 2;
    unsigned long value = 0;
    unsigned char c = 0;

    for (c = 7; c >= 4; c--)
    {
        value <<= 8;
        value |= data[c];
    }

    value /= factor;

    //Serial.print("Fuel C: ");
    //Serial.println(value, DEC);

    //BDR - Segmentation Fault
    canFMSPointer->fuelConsumption = value;
}

void CanFMSParseFuelLevel(unsigned char *data)
{
    unsigned long value = 0;

    value = data[1];
    value = (value * 4) / 10;

    //Serial.print("Fuel L: ");
    //Serial.println(value, DEC);

    canFMSPointer->fuelLevel = value;
}

void CanFMSParseEngineTemperature(unsigned char *data)
{
    const unsigned int factor = -40;

    canFMSPointer->engineTemperature = data[0];
    canFMSPointer->engineTemperature += factor;

    //Serial.print("Eng Temp: ");
    //Serial.println(canFMSPointer->engineTemperature, DEC);
}

void CanFMSParseAirSupplyPressure(unsigned char *data)
{
    const unsigned int factor = 8;
    unsigned long value = 0;

    value = data[3];
    value <<= 8;
    value |= data[2];

    value *= factor;

    //Serial.print(("Air Supply Pressure: "));
    //Serial.println(value, DEC);

    canFMSPointer->airSupplyPressure = value;
}

void CanFMSParseFuelEconomy(unsigned char *data)
{
    const unsigned int factor = 512;
    unsigned long value = 0;

    value = data[3];
    value <<= 8;
    value |= data[2];

    value /= factor;

    //Serial.print(("Fuel Economy: "));
    //Serial.println( value, DEC);

    canFMSPointer->fuelEconomy = value;
}

void CanFMSParseHighResFuelConsumption(unsigned char *data)
{
    unsigned long value = 0;

    for (char c = 7; c >= 4; c--)
    {
        value <<= 8;
        value |= data[c];
    }

    //Serial.print(("HighRes Fuel Consumption: "));
    //Serial.println( value, DEC);

    canFMSPointer->fuelConsumption = value;
}

void CanFMSParseVehicleSpeed(unsigned char *data)
{
    const unsigned int factor = 256;
    unsigned long value = 0;

    value = data[2];
    value <<= 8;
    value |= data[1];

    value /= factor;

    //Serial.print(("Vehicle Speed: "));
    //Serial.println(value, DEC);

    canFMSPointer->vehicleSpeed = value;
}

void CanFMSParseAlternator(unsigned char *data)
{
    unsigned long value = 0;
    unsigned char c = 0;

    value |= data[0];

    //Serial.print(("Alternator: "));
    //Serial.println( value, DEC);
}

void CanFMSParseGear(unsigned char *data)
{
    char selectedGear = data[0] - 125;
    char currentGear = data[3] - 125;

    unsigned long value;

    value = data[0];
    value <<= 8;
    value |= data[3];

    //Serial.print(("Gears: Selected "));
    //Serial.println( selectedGear, DEC);
    //Serial.print(("Gears: Current "));
    //Serial.println( currentGear, DEC);

    canFMSPointer->gear = value;
}

void CanFMSParseAirSuspension(unsigned char *data)
{
    unsigned char wheels[8];
    unsigned char factor = 10;
    unsigned char c = 0;

    for (c = 0; c < 8; c++)
    {
        wheels[c] = data[c];
        wheels[c] /= factor;
        //		//Serial.print("Air Suspension Pressure %d: %d kPa", c, wheels[c]);
    }
}

void CanFMSParseVehicleWeight(unsigned char *data)
{
    //Eixo 01
    if (data[0] == 0x10)
    {
        canFMSPointer->vehicleWeight = (data[2] << 8) + data[1];
        canFMSPointer->vehicleWeight /= 2;
    }

    //Serial.print(("Vehicle Weight: "));
    //Serial.println( canFMSPointer->vehicleWeight, DEC);
}

void CanFMSParseWindshieldWiper(unsigned char *data)
{
    canFMSPointer->windshieldWiper = (data[0] & 0xF0 == 0x00 ? 0 : 1);

    //Serial.print("Windshield Wiper: ");
    //Serial.println(canFMSPointer->windshieldWiper, DEC);
}

//Passar para o VW
void CanFMSParseBreak(unsigned char *data)
{
    const float factor = 0.4;

    canFMSPointer->breakPosition = data[1] * factor;

    //Serial.print(("Break: "));
    //Serial.println( canFMSPointer->breakPosition, DEC);
}

void CanFMSParseIgnition(unsigned char *data)
{
    canFMSPointer->ignition = data[0];

    //Serial.print("Ignition: ");
    //Serial.println(canFMSPointer->ignition, DEC);
}

void CanFMSParseToxicGases(unsigned char *data)
{
    canFMSPointer->toxicGases = (data[2] << 8) + data[1];

    //Serial.print("Toxic Gases: ");
    //Serial.println( canFMSPointer->toxicGases, DEC);
}

void CanFMSParseNoiseLevel(unsigned char *data)
{
    canFMSPointer->noiseLevel = data[3];

    //Serial.print("Noise Level: ");
    //Serial.println(canFMSPointer->noiseLevel, DEC);
}

void CanFMSParseParticulateMaterial(unsigned char *data)
{
    canFMSPointer->particulateMaterial = data[0];

    //Serial.print("Particulate Material: ");
    //Serial.println(canFMSPointer->particulateMaterial, DEC);
}

void CanFMSParseAirHumidity(unsigned char *data)
{
    canFMSPointer->airHumidity = (data[3] << 8) + data[2];
    canFMSPointer->airHumidity /= 100;

    //Serial.print("Air Humidity: ");
    //Serial.println(canFMSPointer->airHumidity, DEC);
}

void CanFMSParseTirePressure(unsigned char *data)
{
    canFMSPointer->tirePressure = data[1];
    canFMSPointer->tirePressure *= 4;

    //Serial.print("Tire Pressure: ");
    //Serial.println(canFMSPointer->tirePressure, DEC);
}

void CanFMSDataParse(unsigned long canId, unsigned char * data, Stream * serial, struct CANValue &pointer)
{
    unsigned long pgn = (canId & 0x00FFFF00) >> 8;

    ////Serial.print ("FMS Packet: PGN=%04X", pgn);

    canFMSPointer = &pointer;

    switch (pgn)
    {
        case CAN_PACKET_FUEL_CONSUMPTION: 		    CanFMSParseFuelConsumption(data); canFMSPacketReceived = true; break;
        case CAN_PACKET_FUEL_LEVEL: 		        CanFMSParseFuelLevel(data); 		        canFMSPacketReceived = true; break;
        //case CAN_PACKET_TACHOGRAPH: 		        CanFMSParseTachograph(data); 		        break;
        //case CAN_PACKET_HIGHRES_DISTANCE: 		    CanFMSParseHighResDistance(data); 		    break;
        case CAN_PACKET_ENGINE_TEMPERATURE: 	    CanFMSParseEngineTemperature(data); 	    canFMSPacketReceived = true; break;
        case CAN_PACKET_AIR_SUPPLY_PRESSURE: 	    CanFMSParseAirSupplyPressure(data); 	    canFMSPacketReceived = true; break;
        case CAN_PACKET_FUEL_ECONOMY: 		        CanFMSParseFuelEconomy(data); 		        canFMSPacketReceived = true; break;
        case CAN_PACKET_HIGHRES_FUEL_CONSUMPTION: 	CanFMSParseHighResFuelConsumption(data); 	canFMSPacketReceived = true; break;
        //case CAN_PACKET_VEHICLE_SPEED: 		        CanFMSParseVehicleSpeed(data); 		        canFMSPacketReceived = true; break;
        //case CAN_PACKET_DATE_TIME: 			        CanFMSParseDateTime(data); 		        break;
        case CAN_PACKET_ALTERNATOR: 		        CanFMSParseAlternator(data); 		        canFMSPacketReceived = true; break;
        case CAN_PACKET_GEAR: 			            CanFMSParseGear(data); 			            canFMSPacketReceived = true; break;
        case CAN_PACKET_AIR_SUSPENSION: 		    CanFMSParseAirSuspension(data); 		    canFMSPacketReceived = true; break;

        case CAN_PACKET_VEHICLE_WEIGHT: 		    CanFMSParseVehicleWeight(data); 		    canFMSPacketReceived = true; break;

        //case CAN_PACKET_AIR_CONDITIONING:		    CanFMSParseTellTale(data);                  canFMSPacketReceived = true; break;
        case CAN_PACKET_WINDSHIELD_WIPER:		    CanFMSParseWindshieldWiper(data);           canFMSPacketReceived = true; break;
        case CAN_PACKET_BREAK:			            CanFMSParseBreak(data); 			        canFMSPacketReceived = true; break;
        case CAN_PACKET_IGNITION:			        CanFMSParseIgnition(data); CanFMSParseNoiseLevel(data); CanFMSParseToxicGases(data); canFMSPacketReceived = true; break;

        case FMS_PARTICULATE_MATERIAL:		        CanFMSParseParticulateMaterial(data); 	    canFMSPacketReceived = true; break;

        case CAN_PACKET_TIRE_PRESSURE:              CanFMSParseTirePressure(data);              canFMSPacketReceived = true; break;

        case FMS_AIR_HUMIDITY:                      CanFMSParseAirHumidity(data);               canFMSPacketReceived = true; break;

        default:
            ////Serial.print("Packet not recognized %d", pgn);
            break;
    }
}

bool CanFMSPacketReceived() {
  return canFMSPacketReceived;
}

void CanFMSResetPacketReceived() {
  canFMSPacketReceived = false;
}

#endif
