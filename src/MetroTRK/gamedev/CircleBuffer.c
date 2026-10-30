#include <MetroTRK.h>

#include <string.h>

void MWEnterCriticalSection(void* criticalSection);
void MWExitCriticalSection(void* criticalSection);
void MWInitializeCriticalSection(void* criticalSection);

typedef struct CircleBuffer {
    u8* readPtr;
    u8* writePtr;
    u8* buffer;
    u32 bufferSize;
    u32 bytesToRead;
    u32 bytesToWrite;
    void* criticalSection;
} CircleBuffer;

u32 CBGetBytesAvailableForRead(CircleBuffer* circleBuffer) {
    return circleBuffer->bytesToRead;
}

void CircleBufferInitialize(CircleBuffer* circleBuffer, u8* buffer,
                            u32 bufferSize) {
    circleBuffer->buffer = buffer;
    circleBuffer->bufferSize = bufferSize;
    circleBuffer->readPtr = circleBuffer->buffer;
    circleBuffer->writePtr = circleBuffer->buffer;
    circleBuffer->bytesToRead = 0;
    circleBuffer->bytesToWrite = circleBuffer->bufferSize;

    MWInitializeCriticalSection(&circleBuffer->criticalSection);
}

int CircleBufferWriteBytes(CircleBuffer* circleBuffer, const void* bytes,
                           u32 length) {
    u32 bytesAvailable;

    if (length > circleBuffer->bytesToWrite) {
        return -1;
    }

    MWEnterCriticalSection(&circleBuffer->criticalSection);
    bytesAvailable =
        circleBuffer->bufferSize -
        (circleBuffer->writePtr - circleBuffer->buffer);

    if (bytesAvailable >= length) {
        memcpy(circleBuffer->writePtr, bytes, length);
        circleBuffer->writePtr += length;
    } else {
        memcpy(circleBuffer->writePtr, bytes, bytesAvailable);
        memcpy(circleBuffer->buffer, (const u8*)bytes + bytesAvailable,
               length - bytesAvailable);
        circleBuffer->writePtr =
            circleBuffer->buffer + length - bytesAvailable;
    }

    if (circleBuffer->bufferSize ==
        circleBuffer->writePtr - circleBuffer->buffer) {
        circleBuffer->writePtr = circleBuffer->buffer;
    }

    circleBuffer->bytesToWrite -= length;
    circleBuffer->bytesToRead += length;

    MWExitCriticalSection(&circleBuffer->criticalSection);
    return 0;
}

int CircleBufferReadBytes(CircleBuffer* circleBuffer, void* bytes, u32 length) {
    u32 bytesAvailable;

    if (length > circleBuffer->bytesToRead) {
        return -1;
    }

    MWEnterCriticalSection(&circleBuffer->criticalSection);
    bytesAvailable =
        circleBuffer->bufferSize -
        (circleBuffer->readPtr - circleBuffer->buffer);

    if (length < bytesAvailable) {
        memcpy(bytes, circleBuffer->readPtr, length);
        circleBuffer->readPtr += length;
    } else {
        memcpy(bytes, circleBuffer->readPtr, bytesAvailable);
        memcpy((u8*)bytes + bytesAvailable, circleBuffer->buffer,
               length - bytesAvailable);
        circleBuffer->readPtr =
            circleBuffer->buffer + length - bytesAvailable;
    }

    if (circleBuffer->bufferSize ==
        circleBuffer->readPtr - circleBuffer->buffer) {
        circleBuffer->readPtr = circleBuffer->buffer;
    }

    circleBuffer->bytesToWrite += length;
    circleBuffer->bytesToRead -= length;

    MWExitCriticalSection(&circleBuffer->criticalSection);
    return 0;
}
