#include <revolution/OS.h>
#include <revolution/WPAD.h>

WPADMemBlock _wmb[WPAD_MAX_CONTROLLERS];

void ResultCallback(s32 chan, s32 result) {
    WPADCallback callback;

    _wmb[chan].at_0x00 = 0;
    callback = _wmb[chan].at_0x10;
    _wmb[chan].at_0x10 = NULL;

    if (callback != NULL) {
        callback(chan, result);
    }
}

s32 WPADReadFaceData(s32 chan, void* dst, u16 size, u32 src, WPADCallback cb) {
    s32 result;
    BOOL enabled;
    BOOL handshakeFinished;
    WPADCB* p = _wpdcb[chan];

    enabled = OSDisableInterrupts();
    result = p->status;
    handshakeFinished = p->handshakeFinished;

    if (result != WPAD_ERR_NO_CONTROLLER) {
        if (handshakeFinished == FALSE) {
            result = WPAD_ERR_BUSY;
        } else {
            if (_wmb[chan].at_0x00 == 0) {
                _wmb[chan].at_0x00 = 1;
                _wmb[chan].at_0x10 = cb;
                OSRestoreInterrupts(enabled);

                return WPADReadMemoryAsync(chan, dst, size, src + 0xfca,
                                           ResultCallback);
            }

            result = WPAD_ERR_BUSY;
        }
    }

    OSRestoreInterrupts(enabled);

    if (cb != NULL) {
        cb(chan, result);
    }

    return result;
}

s32 WPADReadMemoryAsync(s32 chan, void* dst, u16 len, u32 addr,
                        WPADCallback cb) {
    WPADCB* p = _wpdcb[chan];
    BOOL handshakeFinished;
    s32 result;
    BOOL enabled;

    enabled = OSDisableInterrupts();
    result = p->status;
    handshakeFinished = p->handshakeFinished;
    OSRestoreInterrupts(enabled);

    if (result != WPAD_ERR_NO_CONTROLLER) {
        if (handshakeFinished == FALSE) {
            result = WPAD_ERR_BUSY;
        } else if (WPADiSendReadData(&p->stdCmdQueue, dst, len, addr, cb)) {
            result = WPAD_ERR_OK;
        } else {
            result = WPAD_ERR_BUSY;
        }
    }

    if (result != WPAD_ERR_OK && cb != NULL) {
        cb(chan, result);
    }

    return result;
}
