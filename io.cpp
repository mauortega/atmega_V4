/* ********************** Mapeamento de IO **********************
    0 - RX0 - 485 (DEFAULT) - MODEM (LOW COST)
    1 - TX0 - 485 (DEFAULT) - MODEM (LOW COST)
    2 - INPUT - INT0 - PCF8574 Interrupt - Chip 01
    3 - INPUT - INT1 - CAN Interrupt
    4 - IO - TX1 - RASP - SOFTWARE SERIAL
    5 - IO - RX1 - RASP - SOFTWARE SERIAL
    6 - IO - GPS_TX - ATRX - SOFTWARE SERIAL
    7 - IO - GPS_RX - ATTX - SOFTWARE SERIAL
    8 - CAN - CS
    9 - RFID - RF_RST_PIN
    10 - RFID - RF_CS_PIN
    11 - MOSI
    12 - MISO
    13 - SCK - LED

    AN0 - Entrada AN1
    AN1 - Entrada AN2
    AN2 - Chaveamento RASP / Display
    AN3 - 485 - RW
    AN4 - GPS / PCF - SDA
    AN5 - GPS / PCF - SCL
    AN6 - Utilizar somente como entrada - Entrada da fonte (10K / 91K)
    AN7 - Utilizar somente como entrada - Entrada da bateria (10K / 91K)

    Opção - PCF8574 - Chip 01 - INPUTS
    0 - RPM
    1 - Odômetro
    2 - Pânico
    3 - Ignição
    4 - INPUT 01
    5 - INPUT 02
    6 - INPUT 03
    7 - INPUT 04

    Opção - PCF8574 - Chip 02 - OUTPUTS
    0 - OUTPUT 1
    1 - OUTPUT 2
    2 - Bloqueio
    3 - Sirene
    4 -
    5 - Modem Ignition (somente versão LIGHT) - Verificar resistores na placa
    6 - Modem Reset (somente versão LIGHT) - Verificar resistores na placa
    7 - GPS Reset (somente versão LIGHT) - Verificar resistores na placa

2025/02/26 - Modo de SoftPowerOff para Raspberry

*/

#include "io.h"

#include "PCF8574.h"
#include <Wire.h>
#include <avr/wdt.h>

bool decoder01Working = false;
bool decoder02Working = false;

int DECODER_01_ADDR = 0x00;
int DECODER_02_ADDR = 0x00;

PCF8574 Decoder01(DECODER_01_ADDR);
PCF8574 Decoder02(DECODER_02_ADDR);

uint8_t decoder01Inputs;

bool outputs = false;

unsigned long lastWatchDog = 0;

IOStruct ioData;

// Global soft power off enableb
bool softPowerOffEnabled = true;

// Enable only if vehycle has 24 V battery
bool softPowerOffVoltageEnabled = false;

void IOSetup()
{

  Wire.begin();

  if (Wire.requestFrom(0x38, 1))
  {
    DECODER_01_ADDR = 0x38;
    Decoder01 = PCF8574(DECODER_01_ADDR);
  }
  else if (Wire.requestFrom(0x20, 1))
  {
    DECODER_01_ADDR = 0x20;
    Decoder01 = PCF8574(DECODER_01_ADDR);
  }
  else
  {
    Serial.println(F("Decoder01 not found"));
  }

  if (Wire.requestFrom(0x39, 1))
  {
    DECODER_02_ADDR = 0x39;
    Decoder02 = PCF8574(DECODER_02_ADDR);
  }
  else if (Wire.requestFrom(0x21, 1))
  {
    DECODER_02_ADDR = 0x21;
    Decoder02 = PCF8574(DECODER_02_ADDR);
  }
  else
  {
    Serial.println(F("Decoder02 not found"));
  }

  pinMode(RASP_POWER, OUTPUT);
  RaspberryON();
}

void RaspberryON()
{
  digitalWrite(RASP_POWER, true);
}

void RaspberryOFF()
{
  digitalWrite(RASP_POWER, false);
}

/*
void IOLoop() {
  decoder01Inputs = Decoder01.read8();

  ioData.ignition = GetDecoderInputDebounce(DECODER_IGN);
  ioData.panic = GetDecoderInputDebounce(DECODER_PAN);

  ioData.externalAnalogBattery = analogRead(ANALOG_01);
  ioData.externalAnalogAlternator = analogRead(ANALOG_02);
  ioData.internalAnalogPower = analogRead(ANALOG_PWR);
  ioData.internalAnalogBattery = analogRead(ANALOG_BAT);
  ioData.temperature = (unsigned int)(GetTemperature() * 10);
}
*/

VoltageFallState voltageFallState = VFNormalState;
VoltageRiseState voltageRiseState = VRNormalState;

unsigned long voltageFallTimer = 0;
unsigned long voltageRiseTimer = 0;
bool raspberryShutdown = false;
bool raspberryOff = false;

#define VoltageFactor 30

double ToVoltage(unsigned int analogReading)
{
  double result = 0.03215;
  result *= analogReading;
  result += 1.4178;

  return result;
}

void IOLoop(Stream *serial)
{
  if (voltageFallState == VFNormalState)
  {
    decoder01Inputs = Decoder01.read8();

    ioData.ignition = GetDecoderInputDebounce(DECODER_IGN);
    ioData.panic = GetDecoderInputDebounce(DECODER_PAN);
  }

  ioData.externalAnalogBattery = analogRead(ANALOG_01);
  ioData.externalAnalogAlternator = analogRead(ANALOG_02);
  ioData.internalAnalogPower = analogRead(ANALOG_PWR);
  ioData.internalAnalogBattery = analogRead(ANALOG_BAT);
  ioData.temperature = (unsigned int)(GetTemperature() * 10);

  unsigned long currentMillis = millis();

  // serial->print("[IODebug] Voltage: ");
  // serial->println(ioData.internalAnalogPower, DEC);

  // Verificar se a tensão caiu abaixo de 18V
  if (softPowerOffEnabled && softPowerOffVoltageEnabled && ToVoltage(ioData.internalAnalogPower) < 18.0)
  {
    if (voltageFallState == VFNormalState)
    {
      voltageFallState = UnderVoltageState;
      voltageFallTimer = currentMillis;
      voltageRiseState = VRNormalState;
      // serial->println("[IODebug] Voltage < 18 - Undervoltage");
    }

    // 30s -> Desativar PCF
    if ((currentMillis - voltageFallTimer >= 30000) && (voltageFallState == UnderVoltageState))
    {
      Wire.end();
      voltageFallState = PCFDisabledState;
      // serial->println("[IODebug] 30s - PCFDisabled");
    }

    // 45s -> Definir flag de desligamento do Raspberry
    if ((currentMillis - voltageFallTimer >= 45000) && (voltageFallState == PCFDisabledState))
    {
      raspberryShutdown = true;
      voltageFallState = RaspberryShutdownState;
      // serial->println("[IODebug] 45s - RaspberryShutdown");
    }

    // 60s -> Desligar Raspberry
    if ((currentMillis - voltageFallTimer >= 60000) && (voltageFallState == RaspberryShutdownState))
    {
      raspberryOff = true;

      // ToDo: Production
      RaspberryOFF();
      voltageFallState = RaspberryOFFState;
      // serial->println("[IODebug] 60s - RaspberryOFF");
    }
  }

  // Se a tensão subir acima de 20V
  if (ToVoltage(ioData.internalAnalogPower) > 20.0)
  {

    if (softPowerOffVoltageEnabled == false)
    {
      softPowerOffVoltageEnabled = true;
      // serial->println("[IODebug] > 20V - softPowerOffVoltageEnabled = true");
    }

    if (voltageFallState == UnderVoltageState)
    {
      voltageFallState = VFNormalState;
      voltageRiseState = VRNormalState;
      // serial->println("[IODebug] UnderVoltageState => NormalState");
    }

    if (voltageFallState == PCFDisabledState)
    {
      Wire.begin();
      voltageFallState = VFNormalState;
      // serial->println("[IODebug] PCFDisabledState => NormalState");
    }

    if (voltageFallState != VFNormalState && voltageRiseState == VRNormalState)
    {
      voltageRiseState = OverVoltageState;
      voltageRiseTimer = currentMillis;
      // serial->println("[IODebug] > 20V - OverVoltage");
    }

    if (voltageRiseState == OverVoltageState && (currentMillis - voltageRiseTimer >= 60000))
    {
      if (voltageFallState == RaspberryShutdownState || voltageFallState == RaspberryOFFState)
      {

        // ToDo: Production
        RaspberryReset();
        // serial->println("[IODebug] 60s - RaspberryReset");
      }

      voltageFallState = VFNormalState;
      voltageRiseState = VRNormalState;

      Wire.begin();

      // serial->println("[IODebug] voltageRiseState = Normal");
    }
  }
}

void RaspberryReset()
{
  if (softPowerOffVoltageEnabled == false || ToVoltage(ioData.internalAnalogPower) > 20.0)
  {
    RaspberryOFF();

    for (byte i = 0; i < 10; i++)
    {
      delay(1000);
      wdt_reset();
    }

    RaspberryON();

    lastWatchDog = millis();
  }
}

unsigned char GetIOInputs()
{
  return decoder01Inputs;
}

bool GetDecoderInput(unsigned char input)
{
  byte value = (decoder01Inputs & input);

  if (value == 0)
    return false;
  else
    return true;
}

// Define constants for return values
const byte ERROR_CODE = 0xFF;
const byte FALSE_CODE = 0;
const byte TRUE_CODE = 1;

byte GetDecoderInputDebounce(unsigned char input)
{
  int errorCode = 0;

  // Read the first value and check for errors
  byte value1 = Decoder01.read8();

  if (Decoder01.lastError() == PCF8574_I2C_ERROR)
    return ERROR_CODE;

  value1 &= input;

  // delay(100);

  // Read the second value and check for errors
  // byte value2 = Decoder01.read8();

  // if (Decoder01.lastError() == PCF8574_I2C_ERROR)
  //   return ERROR_CODE;

  // value2 &= input;

  // Determine the return value based on the readings
  // if (value1 == 0 && value2 == 0)
  if (value1 == 0)
    return 0;

  // if (value1 == input && value2 == input)
  if (value1 == input)
    return 1;

  return ERROR_CODE;
}

// ********************** Check I2C Addresses **********************

void CheckI2CAddresses(Stream *serial)
{
  byte error;
  byte address;

  Wire.begin();

  // serial->println("[I2C] Address Checker");

  for (address = 1; address < 127; address++)
  {
    // serial->print("[I2C] Checking address: ");
    // serial->println(address, DEC);

    Wire.beginTransmission(address);

    error = Wire.endTransmission(true);

    if (error == 0)
    {
      if (address == DECODER_01_ADDR)
        decoder01Working = true;

      if (address == DECODER_02_ADDR)
        decoder02Working = true;

      // serial->print("[I2C] Device found at address 0x");

      // serial->println(address, HEX);
    }
    else
    {
      if (error == 4)
      {
        /*
                serial->print("[I2C] Unknown error at address 0x");

                if (address < 16)
                    serial->print("0");

                serial->println(address, HEX);
                */
      }
    }

    if (address < 16)
      serial->print(error + 0x30);
  }

  serial->println(F("[I2C] Address Checker Finished"));
}

bool DecoderWorking(byte decoder)
{
  switch (decoder)
  {
  case 1:
    return decoder01Working;
    break;
  case 2:
    return decoder02Working;
    break;
  }

  return false;
}

double GetTemperature(void)
{
  unsigned int wADC;
  double t;

  // The internal temperature has to be used
  // with the internal reference of 1.1V.
  // Channel 8 can not be selected with
  // the analogRead function yet.

  // Set the internal reference and mux.
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
  ADCSRA |= _BV(ADEN); // enable the ADC

  delay(20); // wait for voltages to become stable.

  ADCSRA |= _BV(ADSC); // Start the ADC

  // Detect end-of-conversion
  while (bit_is_set(ADCSRA, ADSC))
    ;

  // Reading register "ADCW" takes care of how to read ADCL and ADCH.
  wADC = ADCW;

  // The offset of 324.31 could be wrong. It is just an indication.
  t = (wADC - 324.31) / 1.22;

  // The returned temperature is in degrees Celsius.
  return (t);
}

struct IOStruct IOGetData()
{
  return ioData;
}

void SoftPowerOff(bool enabled)
{
  softPowerOffEnabled = enabled;
}

byte GetShutdownState()
{
  return voltageFallState;
}