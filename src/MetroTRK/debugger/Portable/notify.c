#include <MetroTRK.h>

typedef enum {
    DSMSG_NotifyStop = 0x90,
    DSMSG_NotifyException = 0x91,
} DSMessageID;

DSError TRKDoNotifyStopped(DSMessageID event) {
    int reply;
    int buffer;
    TRKMessageBuffer* message;
    DSError err;

    err = TRKGetFreeBuffer(&buffer, &message);
    if (err == kNoError) {
        if (err == kNoError) {
            if (event == DSMSG_NotifyStop) {
                TRKTargetAddStopInfo(message);
            } else {
                TRKTargetAddExceptionInfo(message);
            }
        }

        err = TRKRequestSend(message, &reply, 2, 3, 1);
        if (err == kNoError) {
            TRKReleaseBuffer(reply);
        }

        TRKReleaseBuffer(buffer);
    }

    return err;
}
