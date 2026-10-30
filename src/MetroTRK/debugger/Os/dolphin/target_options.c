#include <types.h>

u8 bUseSerialIO[8];

void SetUseSerialIO(u8 useSerialIO) {
    bUseSerialIO[0] = useSerialIO;
}

u8 GetUseSerialIO(void) {
    return bUseSerialIO[0];
}
