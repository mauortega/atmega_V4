#ifndef CAN_H
#define CAN_H

#include <Arduino.h>
#include "src/mcp_can.h"
#include "src/mcp_can_dfs.h"

#define CAN_DEBUG

#define CAN_CS_PIN 8
#define CAN0_INT 3

// #define CAN_FMS

#define CAN_NOT_DETECTED 0x00
#define CAN_125K 0x50  // Caracter P = 80
#define CAN_250K 0x51  // Caracter Q = 81
#define CAN_500K 0x52  // Caracter R = 82
#define CAN_1000K 0x53 // Caracter S = 83
#define CAN_MAX_BAUD 0x54

#pragma pack(1)

enum CAN_VERSIONS
{
    CAN_VW_VER1,
    CAN_MB_VER1,
    CAN_FMS_VER1,
    CAN_VW_VER2,
    CAN_MB_VER2,
    CAN_FMS_VER2
};

// #ifndef CAN_FMS
struct CANValue
{
    unsigned char protocolVersion;
    unsigned long timeStamp;
    unsigned int engineSpeed;
    unsigned long fuelConsumption;
    unsigned int fuelLevel;
    unsigned long engineHours;
    unsigned int tachograph;
    unsigned long highResDistance;
    signed char engineTemperature;
    unsigned int ambientTemperature; // Sinal ?
    unsigned int airSupplyPressure;
    unsigned int fuelEconomy;
    unsigned int vehicleSpeed;
    int vehicleSpeedDiff;
    unsigned char pedalPosition;
    unsigned char doorsControl1;
    unsigned char doorsControl2;
    unsigned char alternator;
    char gear;
    unsigned int oilPressure;
    unsigned int fuelPressure;
    unsigned char parkingBreak;
    unsigned char breakSwitch;
    unsigned char waterInFuel;
    unsigned char handbrake;
    unsigned char footbrake;
    unsigned char retarder;
    unsigned int adblueconsavg;
    unsigned char adbluevol;

    unsigned char clutchSwitch;
    unsigned int cabinTemperature; // Sinal ?

#ifdef CAN_FMS
    unsigned int vehicleWeight;
    unsigned char lights;
    unsigned char airConditioning;
    unsigned char windshieldWiper;
    unsigned char breakPosition;
    unsigned char ignition;
    unsigned char tellTale;
    unsigned int toxicGases;
    unsigned char noiseLevel;
    unsigned int airHumidity;
    unsigned char particulateMaterial;
    unsigned int tirePressure;
#endif
};
/*
#else
struct CANValue
{
    unsigned char protocolVersion;
    unsigned int engineSpeed;
    unsigned long fuelConsumption;
    unsigned int fuelLevel;
    unsigned long engineHours;
    unsigned int tachograph;
    unsigned long highResDistance;
    signed char engineTemperature;
    unsigned int ambientTemperature;
    unsigned int airSupplyPressure;
    unsigned int fuelEconomy;
    unsigned int vehicleSpeed;
    unsigned char pedalPosition;
    unsigned char doorsControl1;
    unsigned char doorsControl2;
    unsigned char alternator;
    char gear;

    unsigned long airSuspension;
    unsigned long vehicleWeight;

    unsigned int oilPressure;

    unsigned long lights;
    unsigned long airConditioning;
    unsigned long windshieldWiper;
    unsigned long acceleration;

    unsigned long breakPosition;

    unsigned long ignition;
    unsigned long tellTale;
    unsigned long noiseLevel;
    unsigned long particulateMaterial;
    unsigned long toxicGases;
    unsigned long airHumidity;

    unsigned int fuelPressure;
    unsigned char parkingBreak;
    unsigned char breakSwitch;
    unsigned char waterInFuel;

    unsigned int handbrake;
    unsigned int footbrake;
    unsigned int retarder;
    //Utiliza somente 1 bit no flag
    unsigned long  adblueconsavg;
    unsigned long  adbluevol;
};
#endif
*/

bool CanSetup();
void CanChangeSpeed(byte newSpeed, Stream *serial);

void CanLoop(Stream *serial);

bool CanSimulatorSend(Stream *serial, unsigned long id, byte *data);
unsigned char *CanGetData(Stream *serial);
unsigned int CanGetDataLen();
bool CanDataReceived();
void CanEmulator(unsigned int value);
bool CanStatus(Stream *serial);
bool CanSimulator(void);
void CanDebug(const char *format, unsigned long value);
void CanDebug(const __FlashStringHelper *format, unsigned long value);
void CanDebug(Stream *serial, const __FlashStringHelper *format, unsigned long value);
void SetCanDebug(bool enabled);

void CanChangeNormalMode(void);
void CanChangeListenMode(void);

void CanSetCanBitRate(byte b);
byte CanGetCanBitRate();
void CanSetListenMode(byte b);
bool CanGetCanStarted();

#endif
