//
// Funcoes de watchdog para raspberry pi
//

#include <Arduino.h>

#include "rpiWdt.h"
#include "io.h"

// WDT - 10 minutes
#define WDTResetRasp 600000L

static bool wdtEnabled = true;
static unsigned long lastWatchDog = 0;

static Stream *serialRpi;

//////////////////////////////////////////////////////////////////
void RpiWdtInit(Stream *serial)
{
  serialRpi = serial;
  lastWatchDog = millis();
}

//////////////////////////////////////////////////////////////////
void RpiWdtLoop()
{
  if (RpiWdtTotalTime() == 0)
  {
    serialRpi->println(F("[RASP] ERROR - Negative Total Time"));
    RpiWdtPulse();
  }

  // Checa se tem que resetar a raspi
  if (wdtEnabled && RpiWdtTotalTime() > WDTResetRasp)
  {
    serialRpi->println(F("[RASP] RESET"));
    RaspberryReset();
    lastWatchDog = millis();
    RpiWdtPulse();
  }
}

//////////////////////////////////////////////////////////////////
bool RpiWtdGetEnabled()
{
  return wdtEnabled;
}

void RpiWtdSetEnabled(bool b)
{
  wdtEnabled = b;
}

//////////////////////////////////////////////////////////////////
unsigned long RpiWdtTotalTime()
{
  unsigned long totalMillis = 0;

  if (millis() >= lastWatchDog)
    totalMillis = millis() - lastWatchDog;

  return totalMillis;
}

void RpiWdtPulse()
{
  lastWatchDog = millis();
}