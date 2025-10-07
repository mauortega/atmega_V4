#ifndef __RPIWDT_H__
#define __RPIWDT_H__

void RpiWdtInit(Stream *serial);
void RpiWdtLoop();

bool RpiWtdGetEnabled();
void RpiWtdSetEnabled(bool b);

void RpiWdtPulse();
unsigned long RpiWdtTotalTime();

#endif