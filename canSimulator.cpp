#include <Arduino.h>

#include "protocol.h"
#include "canSimulator.h"
#include "can.h"

#define CAN_SIMULATOR_BUFF_LEN 100

unsigned char canSimulatorBuffer [CAN_SIMULATOR_BUFF_LEN];
unsigned char canSimulatorPointer;

bool canSimulatorPacketReceived = false;

void CanSimulatorLoop (Stream *serial)
{
    while (serial->available())
    {
        unsigned char data = serial->read();

        if (data == PROTOCOL_START)
            canSimulatorPointer = 0;

        if (canSimulatorPointer < CAN_SIMULATOR_BUFF_LEN)
            canSimulatorBuffer[canSimulatorPointer++] = data;

        if (data == PROTOCOL_STOP)
            canSimulatorPacketReceived = true;
    }

    if (canSimulatorPacketReceived)
    {
        unsigned char decoded [canSimulatorPointer];
        unsigned char decodedLen = ProtocolDecodeBuffer(& canSimulatorBuffer [0], decoded, canSimulatorPointer);

        unsigned long id = (decoded[0] << 8) | decoded [1];

        serial->print(F("[CAN] ID: "));
        serial->print(id, HEX);
        serial->print(F(" VALUE: "));
        
        ProtocoloPrintHex(serial, decoded, decodedLen);

        if (decodedLen == 10)
            CanSimulatorSend (serial, id, & decoded [2]);

        if (decodedLen == 12)
        {
            id <<= 8;
            id |= decoded [2];
            id <<= 8;
            id |= decoded [3];

            CanSimulatorSend (serial, id, & decoded [4]);
        }

        canSimulatorPacketReceived = false;
    }    
}
