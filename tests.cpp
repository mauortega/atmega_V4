#include <Arduino.h>
#include "protocol.h"

#define ProtocolMaxDataLen 200
#define ProtocolMaxEncodedBuffer 400
#define ProtocolMaxDataValue 255

void TestSetup()
{
    randomSeed(analogRead(0));
    Serial.begin(115200);
}

unsigned int TestProtocolEncodeDecode()
{
    unsigned char dataToEncode [ProtocolMaxDataLen];
    unsigned char dataEncoded [ProtocolMaxEncodedBuffer];
    unsigned char dataDecoded [ProtocolMaxDataLen];

    unsigned char randomLen = random(ProtocolMaxDataLen);

    unsigned int errors = 0;

    Serial.print(F("[TEST] Random Packet: "));

    for ( unsigned char c = 0; c < randomLen; c++ )
    {
        dataToEncode[c] = random(ProtocolMaxDataValue);

        if (dataToEncode[c] < 0x10)
            Serial.print(F("0"));

        Serial.print(dataToEncode[c], HEX);
    }

    Serial.print ("\r\n");

    Serial.print(F("[TEST] Packet Encoded: "));

    int encodedPacketLen = ProtocolEncodeBuffer( & dataToEncode [0], & dataEncoded [0], randomLen );

    for ( int c = 0; c < encodedPacketLen; c++ )
    {
        if (dataEncoded[c] < 0x10)
            Serial.print(F("0"));

        Serial.print(dataEncoded[c], HEX);
    }

    Serial.print ("\r\n");

    Serial.print(F("[TEST] Packet Decoded: "));

    int decodedPacketLen = ProtocolDecodeBuffer ( & dataEncoded [0] , &dataDecoded [0], encodedPacketLen );

    for ( int c = 0; c < decodedPacketLen; c++ )
    {
        if (dataDecoded[c] < 0x10)
            Serial.print(F("0"));

        Serial.print(dataDecoded[c], HEX);

        if (dataToEncode[c] != dataDecoded [c] )
            errors ++;
    }

    Serial.print ("\r\n");

    Serial.print ("[TEST] Total Errors: ");
    Serial.print (errors, DEC );
    Serial.print ("\r\n");

    return errors;
}

void TestProtocolAll()
{
    for (int c = 0; c < 1000; c++ )
    {
        if ( TestProtocolEncodeDecode() != 0 )
        {
            Serial.print ("[TEST] TEST ERROR !!! ");
            break;
        }
    }
}

void TestAll()
{
    TestProtocolAll();
}
