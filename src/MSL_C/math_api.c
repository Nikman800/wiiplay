#include <math.h>

#pragma cplusplus on

extern "C" int __signbitd(double x);

int __fpclassifyf(float x) {
    switch ((*(s32*)&x) & 0x7f800000) {
    case 0x7f800000:
        return ((*(s32*)&x) & 0x007fffff) ? FP_NAN : FP_INFINITE;
    case 0:
        return ((*(s32*)&x) & 0x007fffff) ? FP_SUBNORMAL : FP_ZERO;
    }
    return FP_NORMAL;
}

int __signbitd(double x) {
    return (*(s32*)&x) & 0x80000000;
}

int __fpclassifyd(double x) {
    switch ((*(s32*)&x) & 0x7ff00000) {
    case 0x7ff00000:
        if ((*(s32*)&x) & 0x000fffff || *(((s32*)&x) + 1)) {
            return FP_NAN;
        }
        return FP_INFINITE;
    case 0:
        if ((*(s32*)&x) & 0x000fffff || *(((s32*)&x) + 1)) {
            return FP_SUBNORMAL;
        }
        return FP_ZERO;
    }
    return FP_NORMAL;
}
