#include <revolution/OS.h>

typedef volatile u32 vu32;

#define GPIO_SCL 0x8000
#define GPIO_SDA 0x4000
#define I2C_NACK 0x8000

#define I2C_ACK GPIO_SDA
#define I2C_START 0x8000
#define I2C_STOP 0xC000

volatile u32 __i2c_ident_flag = 1;
volatile u32 __i2c_ident_first = 0;

void WaitMicroTime(s32 microTime) {
    s64 startTime = __OSGetSystemTime();

    while (OS_TICKS_TO_USEC(__OSGetSystemTime() - startTime) < microTime) {
        ;
    }
}

BOOL sendSlaveAddr(u8 slaveAddr) {
    s32 count;

    if (__i2c_ident_flag == 0) {
        *(vu32*)0xCD8000C0 = (*(vu32*)0xCD8000C0 & ~I2C_START) | I2C_START;
    } else {
        *(vu32*)0xCD8000C0 &= ~I2C_START;
    }

    WaitMicroTime(2);
    *(vu32*)0xCD8000C0 &= ~GPIO_SDA;

    for (count = 0; count < 8; count++) {
        if ((slaveAddr & 0x80) != 0) {
            if (__i2c_ident_flag == 0) {
                *(vu32*)0xCD8000C0 &= ~I2C_START;
            } else {
                *(vu32*)0xCD8000C0 = (*(vu32*)0xCD8000C0 & ~I2C_START) | I2C_START;
            }
        } else {
            if (__i2c_ident_flag == 0) {
                *(vu32*)0xCD8000C0 = (*(vu32*)0xCD8000C0 & ~I2C_START) | I2C_START;
            } else {
                *(vu32*)0xCD8000C0 &= ~I2C_START;
            }
        }

        WaitMicroTime(2);
        *(vu32*)0xCD8000C0 = (*(vu32*)0xCD8000C0 & ~GPIO_SDA) | GPIO_SDA;
        WaitMicroTime(2);
        slaveAddr <<= 1;
        *(vu32*)0xCD8000C0 &= ~GPIO_SDA;
    }

    *(vu32*)0xCD8000C4 = (*(vu32*)0xCD8000C4 & ~I2C_START) | I2C_ACK;
    WaitMicroTime(2);
    *(vu32*)0xCD8000C0 = (*(vu32*)0xCD8000C0 & ~GPIO_SDA) | GPIO_SDA;
    WaitMicroTime(2);

    if (__i2c_ident_flag == 1 && ((*(vu32*)0xCD8000C8 >> 15) & 1) != 0) {
        return FALSE;
    }

    if (__i2c_ident_flag == 0) {
        *(vu32*)0xCD8000C0 = (*(vu32*)0xCD8000C0 & ~I2C_START) | I2C_START;
    } else {
        *(vu32*)0xCD8000C0 &= ~I2C_START;
    }

    *(vu32*)0xCD8000C4 = (*(vu32*)0xCD8000C4 & ~I2C_START) | I2C_STOP;
    *(vu32*)0xCD8000C0 &= ~GPIO_SDA;
    return TRUE;
}

BOOL __VISendI2CData(u8 slaveAddr, u8* data, s32 length) {
    s32 count;
    u8 value;
    BOOL enabled;

    if (__i2c_ident_first == 0) {
        __i2c_ident_flag = 1;
        __i2c_ident_first = 1;
    }

    enabled = OSDisableInterrupts();

    *(vu32*)0xCD8000C4 = (*(vu32*)0xCD8000C4 & ~I2C_START) | I2C_STOP;
    *(vu32*)0xCD8000C0 = (*(vu32*)0xCD8000C0 & ~GPIO_SDA) | GPIO_SDA;

    if (__i2c_ident_flag == 0) {
        *(vu32*)0xCD8000C0 &= ~I2C_START;
    } else {
        *(vu32*)0xCD8000C0 = (*(vu32*)0xCD8000C0 & ~I2C_START) | I2C_START;
    }

    WaitMicroTime(2);
    WaitMicroTime(2);

    if (!sendSlaveAddr(slaveAddr)) {
        OSRestoreInterrupts(enabled);
        return FALSE;
    }

    *(vu32*)0xCD8000C4 = (*(vu32*)0xCD8000C4 & ~I2C_START) | I2C_STOP;

    while (length != 0) {
        value = *data++;

        for (count = 0; count < 8; count++) {
            if ((value & 0x80) != 0) {
                if (__i2c_ident_flag == 0) {
                    *(vu32*)0xCD8000C0 &= ~I2C_START;
                } else {
                    *(vu32*)0xCD8000C0 = (*(vu32*)0xCD8000C0 & ~I2C_START) | I2C_START;
                }
            } else {
                if (__i2c_ident_flag == 0) {
                    *(vu32*)0xCD8000C0 = (*(vu32*)0xCD8000C0 & ~I2C_START) | I2C_START;
                } else {
                    *(vu32*)0xCD8000C0 &= ~I2C_START;
                }
            }

            WaitMicroTime(2);
            *(vu32*)0xCD8000C0 = (*(vu32*)0xCD8000C0 & ~GPIO_SDA) | GPIO_SDA;
            WaitMicroTime(2);
            value <<= 1;
            *(vu32*)0xCD8000C0 &= ~GPIO_SDA;
        }

        *(vu32*)0xCD8000C4 = (*(vu32*)0xCD8000C4 & ~I2C_START) | I2C_ACK;
        WaitMicroTime(2);
        *(vu32*)0xCD8000C0 = (*(vu32*)0xCD8000C0 & ~GPIO_SDA) | GPIO_SDA;
        WaitMicroTime(2);

        if (__i2c_ident_flag == 1 && ((*(vu32*)0xCD8000C8 >> 15) & 1) != 0) {
            OSRestoreInterrupts(enabled);
            return FALSE;
        }

        if (__i2c_ident_flag == 0) {
            *(vu32*)0xCD8000C0 = (*(vu32*)0xCD8000C0 & ~I2C_START) | I2C_START;
        } else {
            *(vu32*)0xCD8000C0 &= ~I2C_START;
        }

        *(vu32*)0xCD8000C4 = (*(vu32*)0xCD8000C4 & ~I2C_START) | I2C_STOP;
        *(vu32*)0xCD8000C0 &= ~GPIO_SDA;
        length--;
    }

    *(vu32*)0xCD8000C4 = (*(vu32*)0xCD8000C4 & ~I2C_START) | I2C_STOP;

    if (__i2c_ident_flag == 0) {
        *(vu32*)0xCD8000C0 = (*(vu32*)0xCD8000C0 & ~I2C_START) | I2C_START;
    } else {
        *(vu32*)0xCD8000C0 &= ~I2C_START;
    }

    WaitMicroTime(2);
    *(vu32*)0xCD8000C0 = (*(vu32*)0xCD8000C0 & ~GPIO_SDA) | GPIO_SDA;
    WaitMicroTime(2);

    if (__i2c_ident_flag == 0) {
        *(vu32*)0xCD8000C0 &= ~I2C_START;
    } else {
        *(vu32*)0xCD8000C0 = (*(vu32*)0xCD8000C0 & ~I2C_START) | I2C_START;
    }

    OSRestoreInterrupts(enabled);
    return TRUE;
}
