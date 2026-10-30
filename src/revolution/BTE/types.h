#ifndef TYPES_H
#define TYPES_H

// BTE build only: this tree's include/types.h, except that TRUE and FALSE are macros, as in
// Petari's revolution/types.h (its BTE headers #define them before any SDK header is read).

#ifdef __cplusplus
#include <cstdarg>
#include <cstddef>
#include <new>
#else
#include <stdarg.h>
#include <stddef.h>
#endif

#include <decomp/compat.h>
#include <decomp/decomp.h>
#include <decomp/lang.h>

#include <macros.h>

typedef unsigned long long u64;
typedef signed long long s64;

typedef unsigned long u32;
typedef signed long s32;

typedef unsigned short u16;
typedef signed short s16;

typedef unsigned char u8;
typedef signed char s8;

typedef float f32;
typedef double f64;

typedef int UNKWORD;
typedef void UNKTYPE;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
typedef int BOOL;

typedef void (*funcptr_t)(void);


// Petari's revolution/types.h additions the BTE sources use
typedef volatile u8 vu8;
typedef volatile u16 vu16;
typedef volatile u32 vu32;
typedef volatile u64 vu64;
typedef volatile s8 vs8;
typedef volatile s16 vs16;
typedef volatile s32 vs32;
typedef volatile s64 vs64;
typedef volatile f32 vf32;
typedef volatile f64 vf64;

#ifndef NO_INLINE
#if __MWERKS__ >= 0x3000
#define ALWAYS_INLINE __attribute__((always_inline))
#define NO_INLINE __attribute__((noinline))
#else
#define ALWAYS_INLINE
#define NO_INLINE
#endif
#endif
#ifndef ATTRIBUTE_ALIGN
#define ATTRIBUTE_ALIGN(num) __attribute__((aligned(num)))
#endif
#ifndef ATTRIBUTE_PACKED
#define ATTRIBUTE_PACKED __attribute__((packed))
#endif
#ifndef ATTRIBUTE_WEAK
#define ATTRIBUTE_WEAK __attribute__((weak))
#endif
#ifndef AT_ADDRESS
#define AT_ADDRESS(x) : x
#endif

// Petari's OS time types (this tree's OS headers use s64/u32 directly)
typedef s64 OSTime;
typedef u32 OSTick;

// Petari's revolution/os.h compiles the SDK's OSAssert_Line out of release builds
#ifndef OSAssert_Line
#define OSAssert_Line(line_, exp_)
#endif

// Petari's revolution/os.h tick conversions, on this tree's OS_TIME_SPEED (revolution/OS/OSTime.h)
#ifndef OSMillisecondsToTicks
#define OSNanosecondsToTicks(nsec) (((nsec) * (OS_TIME_SPEED / 125000)) / 8000)
#define OSTicksToSeconds(ticks) ((ticks) / OS_TIME_SPEED)
#define OSTicksToMilliseconds(ticks) ((ticks) / (OS_TIME_SPEED / 1000))
#define OSTicksToMicroseconds(ticks) (((ticks) * 8) / (OS_TIME_SPEED / 125000))
#define OSSecondsToTicks(sec) ((sec) * OS_TIME_SPEED)
#define OSMillisecondsToTicks(msec) ((msec) * (OS_TIME_SPEED / 1000))
#define OSMicrosecondsToTicks(usec) (((usec) * (OS_TIME_SPEED / 125000)) / 8)
#endif

#endif
