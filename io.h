#ifndef IO_H
#define IO_H

#include <Arduino.h>

#define DECODER_RPM 0x01
#define DECODER_ODO 0x02
#define DECODER_PAN 0x04
#define DECODER_IGN 0x08
#define DECODER_I01 0x10
#define DECODER_I02 0x20
#define DECODER_I03 0x40
#define DECODER_I04 0x80

#define ANALOG_01 0
#define ANALOG_02 1
#define ANALOG_PWR 6
#define ANALOG_BAT 7

// ANALOG 2 = D16
#define RASP_POWER 16

enum VoltageFallState
{
  VFNormalState,
  UnderVoltageState,
  PCFDisabledState,
  RaspberryShutdownState,
  RaspberryOFFState
};
enum VoltageRiseState
{
  VRNormalState,
  OverVoltageState,
  VoltageOKState,
  RaspberryPowerOnState
};

struct IOStruct
{
  unsigned char ignition;
  unsigned char panic;
  unsigned int internalAnalogBattery;
  unsigned int internalAnalogPower;
  unsigned int externalAnalogBattery;
  unsigned int externalAnalogAlternator;
  unsigned int temperature;
};

bool GetDecoderInput(unsigned char input);
byte GetDecoderInputDebounce(unsigned char input);

void IOSetup();
void IOLoop(Stream *serial);
// void CheckI2C();
unsigned char GetIOInputs();

void RaspberryReset();
void RaspberryON();
void RaspberryOFF();

void CheckI2CAddresses(Stream *serial);

bool DecoderWorking(byte decoder);

double GetTemperature(void);

struct IOStruct IOGetData();

void SoftPowerOff(bool enabled);

byte GetShutdownState();

#endif
