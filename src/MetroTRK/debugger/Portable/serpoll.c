#include <MetroTRK.h>

extern int TRKPollUART(void);
extern UARTError TRKReadUARTN(void* dst, size_t size);

unsigned int gTRKFramingState[6];

int TRKTestForPacket(void) {
    unsigned char data[0x880];
    int header[16];
    int id;
    TRKMessageBuffer* buffer;
    int result;

    if (TRKPollUART() <= 0) {
        return -1;
    }

    result = TRKGetFreeBuffer(&id, &buffer);
    TRKSetBufferPosition(buffer, 0);

    if (TRKReadUARTN(header, 0x40) == kUARTNoError) {
        TRKAppendBuffer_ui8(buffer, (unsigned char*)header, 0x40);
        result = id;
        if (header[0] - 0x40 > 0) {
            if (TRKReadUARTN(data, header[0] - 0x40) == kUARTNoError) {
                TRKAppendBuffer_ui8(buffer, data, header[0]);
            } else {
                TRKReleaseBuffer(result);
                result = -1;
            }
        }
    } else {
        TRKReleaseBuffer(result);
        result = -1;
    }

    return result;
}

void TRKGetInput(void) {
    TRKEvent event;
    int bufferIndex;

    bufferIndex = TRKTestForPacket();

    if (bufferIndex != -1) {
        TRKGetBuffer(bufferIndex);
        TRKConstructEvent(&event, kEventMessage);
        gTRKFramingState[0] = -1;
        event.buffer = bufferIndex;
        TRKPostEvent(&event);
    }
}

void TRKProcessInput(int bufferIndex) {
    TRKEvent event;

    TRKConstructEvent(&event, kEventMessage);
    gTRKFramingState[0] = -1;
    event.buffer = bufferIndex;
    TRKPostEvent(&event);
}

DSError TRKInitializeSerialHandler(void) {
    gTRKFramingState[0] = -1;
    gTRKFramingState[2] = 0;
    gTRKFramingState[3] = 0;
    return kNoError;
}

DSError TRKTerminateSerialHandler(void) {
    return kNoError;
}
