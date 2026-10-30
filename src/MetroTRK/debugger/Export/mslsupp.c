#include <internal/file_io.h>
#include <types.h>

#pragma cplusplus on

extern "C" u8 GetUseSerialIO(void);
extern "C" int GetTRKConnected(void);
extern "C" u8 TRKAccessFile(unsigned long command, unsigned long mode,
                            unsigned long* count, unsigned char* buffer);

extern "C" int __read_console(__file_handle file, unsigned char* buffer,
                              unsigned long* count, __idle_proc idle_proc) {
    if (GetUseSerialIO() == 0) {
        return 1;
    }
    if (GetTRKConnected() == 0) {
        return 1;
    }

    {
        struct {
            unsigned long value;
        } transferred;

        transferred.value = *count;
        unsigned char result = TRKAccessFile(0xd1, 0, &transferred.value, buffer);

        *count = transferred.value;
        switch (result) {
        case 0:
            return 0;
        case 2:
            return 2;
        default:
            return 1;
        }
    }

    return 1;
}

int __TRK_write_console(__file_handle file, unsigned char* buffer,
                        unsigned long* count, __idle_proc idle_proc) {
    if (GetUseSerialIO() == 0) {
        return 1;
    }
    if (GetTRKConnected() == 0) {
        return 1;
    }

    {
        struct {
            unsigned long value;
        } transferred;
        transferred.value = *count;
        unsigned char result;
        result = TRKAccessFile(0xd0, 1, &transferred.value, buffer);

        *count = transferred.value;
        switch (result) {
        case 0:
            return 0;
        case 2:
            return 2;
        default:
            return 1;
        }
    }
}
