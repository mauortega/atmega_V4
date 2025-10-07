// Frame.h

#ifndef FRAME_H
#define FRAME_H

#include <Arduino.h>

class Frame {
  public:
    byte Id;
    int16_t Sequence;
    int16_t Length;
    int16_t CRC;

    Frame();
    ~Frame();

    static Frame* deserialize(String frameString);
};

int16_t calculateCRC(byte* data, int length);
void writeInt16BigEndian(byte* buffer, int offset, int16_t value);
int16_t readInt16BigEndian(byte* buffer, int offset);
void FrameSerialize(Stream* serial, Frame* frame, unsigned char *data);

#endif
