#include <MetroTRK.h>

typedef enum {
    DSMSG_ReadFile = 0xD0,
    DSMSG_WriteFile,
    DSMSG_OpenFile,
    DSMSG_CloseFile,
    DSMSG_PositionFile,
} DSMessageID;

typedef struct {
    u32 length;  // 0x0
    u8 command;  // 0x4
    u8 pad0[3];
    union {
        u32 fileHandle;
        u8 openMode;
    };  // 0x8
    union {
        u16 pathLength;
        u16 count;
        u32 position;
    };  // 0xC
    u8 kind;  // 0x10
    u8 pad1[0x2f];
} DSFileMessage;

typedef struct {
    u32 length;      // 0x0
    u8 command;      // 0x4
    u8 pad0[3];
    u32 fileHandle;  // 0x8
    u32 unkC;
    u32 result;      // 0x10
    u16 count;       // 0x14
    u16 pad1;
    u32 position;    // 0x18
} DSFileReply;

extern void OutputData(const void* data, u32 length);
extern int TRKTestForPacket(void);
extern void TRKProcessInput(int buffer);
extern int TRK_strlen(const char* string);

DSError TRKRequestSend(TRKMessageBuffer* request, int* replyID, int mode,
                       int retries, BOOL display);

DSError TRKSuppAccessFile(u32 fileHandle, u8* data, u32* count,
                          int* result, BOOL sendReply, BOOL read) {
    DSFileMessage message;
    int replyID;
    int requestID;
    TRKMessageBuffer* request;
    TRKMessageBuffer* reply;
    u32 chunk;
    u32 transferred;
    int replyResult;
    u16 replyCount;
    BOOL differentCount;
    DSError err;

    if (data == NULL || *count == 0) {
        return 2;
    }

    differentCount = FALSE;
    *result = 0;
    transferred = 0;
    err = kNoError;

    while (differentCount == FALSE && transferred < *count &&
           err == kNoError && *result == 0) {
        TRK_memset(&message, 0, sizeof(message));

        if (*count - transferred > 0x800) {
            chunk = 0x800;
        } else {
            chunk = *count - transferred;
        }

        message.command = read ? DSMSG_WriteFile : DSMSG_ReadFile;
        message.length = read ? sizeof(message) : chunk + sizeof(message);
        message.fileHandle = fileHandle;
        message.count = chunk;

        TRKGetFreeBuffer(&requestID, &request);
        err = TRKAppendBuffer_ui8(request, (const u8*)&message,
                                  sizeof(message));
        if (!read && err == kNoError) {
            err = TRKAppendBuffer_ui8(request, data + transferred, chunk);
        }

        if (err == kNoError) {
            if (sendReply) {
                err = TRKRequestSend(request, &replyID, read ? 5 : 5, 3,
                                     !(read && fileHandle == 0));
                if (err == kNoError) {
                    reply = TRKGetBuffer(replyID);
                }

                replyResult = (u8)((DSFileReply*)reply->data)->result;
                replyCount = ((DSFileReply*)reply->data)->count;
                if (read && err == kNoError && replyCount <= chunk) {
                    TRKSetBufferPosition(reply, 0x40);
                    err = TRKReadBuffer_ui8(reply, data + transferred,
                                            replyCount);
                    if (err == kMsgReadOverflow) {
                        err = kNoError;
                    }
                }

                if (replyCount != chunk) {
                    chunk = replyCount;
                    differentCount = TRUE;
                }
                *result = replyResult;
                TRKReleaseBuffer(replyID);
            } else {
                err = TRKMessageSend(request);
            }
        }

        TRKReleaseBuffer(requestID);
        transferred += chunk;
    }

    *count = transferred;
    return err;
}

DSError TRKRequestSend(TRKMessageBuffer* request, int* replyID, int mode,
                       int retries, BOOL display) {
    DSError err;
    TRKMessageBuffer* reply;
    unsigned int ticks;
    int count;
    int command;
    int replyErr;
    BOOL validReply;

    err = kNoError;
    validReply = TRUE;
    *replyID = -1;

    for (count = retries + 1; count != 0 && *replyID == -1 && err == kNoError;
         count--) {
        err = TRKMessageSend(request);
        if (err == kNoError) {
            if (display) {
                ticks = 0;
            }

            while (TRUE) {
                do {
                    *replyID = TRKTestForPacket();
                } while (*replyID == -1 &&
                         (display == FALSE || ++ticks < 79999980));

                if (*replyID == -1) {
                    break;
                }

                validReply = FALSE;
                reply = TRKGetBuffer(*replyID);
                TRKSetBufferPosition(reply, 0);
                OutputData(reply->data, reply->size);
                command = reply->data[4];
                if (command >= 0x80u) {
                    break;
                }

                TRKProcessInput(*replyID);
                *replyID = -1;
            }

            if (*replyID != -1) {
                if (reply->size < 0x40) {
                    validReply = TRUE;
                }
                if (err == kNoError && !validReply) {
                    replyErr = reply->data[8];
                }
                if (err == kNoError && !validReply &&
                    (command != 0x80 || replyErr != 0)) {
                    validReply = TRUE;
                }
                if (err != kNoError || validReply) {
                    TRKReleaseBuffer(*replyID);
                    *replyID = -1;
                }
            }
        }
    }

    if (*replyID == -1) {
        err = 0x800;
    }
    return err;
}

DSError HandleOpenFileSupportRequest(const char* path, u8 openMode,
                                     u32* fileHandle, u32* result) {
    DSFileMessage message;
    int replyID;
    int requestID;
    TRKMessageBuffer* request;
    DSError err;
    TRKMessageBuffer* reply;

    TRK_memset(&message, 0, sizeof(message));
    *fileHandle = 0;
    message.command = DSMSG_OpenFile;
    message.length = TRK_strlen(path) + sizeof(message) + 1;
    message.openMode = openMode;
    message.pathLength = TRK_strlen(path) + 1;

    TRKGetFreeBuffer(&requestID, &request);
    err = TRKAppendBuffer_ui8(request, (const u8*)&message,
                              sizeof(message));
    if (err == kNoError) {
        err = TRKAppendBuffer_ui8(request, (const u8*)path,
                                  TRK_strlen(path) + 1);
    }

    if (err == kNoError) {
        *result = 0;
        err = TRKRequestSend(request, &replyID, 7, 3, 0);
        if (err == kNoError) {
            reply = TRKGetBuffer(replyID);
        }
        *result = ((DSFileReply*)reply->data)->result;
        *fileHandle = ((DSFileReply*)reply->data)->fileHandle;
        TRKReleaseBuffer(replyID);
    }

    TRKReleaseBuffer(requestID);
    return err;
}

DSError HandleCloseFileSupportRequest(u32 fileHandle, u32* result) {
    DSFileMessage message;
    int replyID;
    int requestID;
    TRKMessageBuffer* request;
    DSError err;
    TRKMessageBuffer* reply;

    TRK_memset(&message, 0, sizeof(message));
    message.command = DSMSG_CloseFile;
    message.length = sizeof(message);
    message.fileHandle = fileHandle;

    err = TRKGetFreeBuffer(&requestID, &request);
    if (err == kNoError) {
        err = TRKAppendBuffer_ui8(request, (const u8*)&message,
                                  sizeof(message));
    }

    if (err == kNoError) {
        *result = 0;
        err = TRKRequestSend(request, &replyID, 3, 3, 0);
        if (err == kNoError) {
            reply = TRKGetBuffer(replyID);
        }
        if (err == kNoError) {
            *result = ((DSFileReply*)reply->data)->result;
        }
        TRKReleaseBuffer(replyID);
    }

    TRKReleaseBuffer(requestID);
    return err;
}

DSError HandlePositionFileSupportRequest(u32 fileHandle, u32* position,
                                         u8 kind, u32* result) {
    DSFileMessage message;
    int replyID;
    int requestID;
    TRKMessageBuffer* request;
    DSError err;
    TRKMessageBuffer* reply;

    TRK_memset(&message, 0, sizeof(message));
    message.command = DSMSG_PositionFile;
    message.length = sizeof(message);
    message.fileHandle = fileHandle;
    message.position = *position;
    message.kind = kind;

    err = TRKGetFreeBuffer(&requestID, &request);
    if (err == kNoError) {
        err = TRKAppendBuffer_ui8(request, (const u8*)&message,
                                  sizeof(message));
    }

    if (err == kNoError) {
        *result = 0;
        *position = (u32)-1;
        err = TRKRequestSend(request, &replyID, 3, 3, 0);
        if (err == kNoError) {
            reply = TRKGetBuffer(replyID);
            if (reply != NULL) {
                *result = ((DSFileReply*)reply->data)->result;
                *position = ((DSFileReply*)reply->data)->position;
            }
        }
        TRKReleaseBuffer(replyID);
    }

    TRKReleaseBuffer(requestID);
    return err;
}
