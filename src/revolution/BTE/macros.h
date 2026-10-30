#ifndef BTE_MACROS_H
#define BTE_MACROS_H

// BTE build only: this tree's include/macros.h plus the helpers Petari's BTE sources expect
// (from Petari's include/macros.h, CC0).
#include "../../../include/macros.h"


// macro helpers

#ifndef STR_
#define STR_(x) #x
#endif
#ifndef STR
#define STR(x) STR_(x)
#endif

// keywords

#ifndef alignas
#ifndef alignas
#define alignas ATTR_ALIGN
#endif
#endif

// attributes

#ifndef ATTR_ALIGN
#define ATTR_ALIGN(x) __attribute__((aligned(x)))
#endif
#ifndef ATTR_FALLTHROUGH
#define ATTR_FALLTHROUGH /* no known attribute, but mwcc doesn't seem to care */
#endif
#ifndef ATTR_UNUSED
#define ATTR_UNUSED __attribute__((unused))
#endif
#ifndef ATTR_WEAK
#define ATTR_WEAK __attribute__((weak))
#endif

// useful stuff

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#ifndef ROUND_DOWN_PTR
#define ROUND_DOWN_PTR(x, align) ((void*)((unsigned long)(x) & -(align)))
#endif

#ifndef POINTER_ADD_TYPE
#define POINTER_ADD_TYPE(type_, ptr_, offset_) ((type_)((unsigned long)(ptr_) + (unsigned long)(offset_)))
#endif
#ifndef POINTER_ADD
#define POINTER_ADD(ptr_, offset_) POINTER_ADD_TYPE(__typeof__(ptr_), ptr_, offset_)
#endif

#ifndef IS_ALIGNED
#define IS_ALIGNED(x, align) (((unsigned long)(x) & ((align) - 1)) == 0)
#endif

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))
#endif

#ifndef BOOLIFY_TERNARY
#define BOOLIFY_TERNARY(expr_) ((expr_) ? 1 : 0)
#endif
#ifndef BOOLIFY_TERNARY_FALSE
#define BOOLIFY_TERNARY_FALSE(expr_) ((expr_) ? 0 : 1)
#endif

// math

#ifndef M_PI_F
#define M_PI_F 3.1415926f
#endif
#ifndef M_TAU
#define M_TAU 6.283185307179586
#endif

#ifndef DEG_TO_RAD_MULT_CONSTANT
#define DEG_TO_RAD_MULT_CONSTANT (M_PI_F / 180.0f)
#endif
#ifndef RAD_TO_DEG_MULT_CONSTANT
#define RAD_TO_DEG_MULT_CONSTANT (180.0f / M_PI_F)
#endif

#ifndef DEG_TO_RAD
#define DEG_TO_RAD(x) ((x) * DEG_TO_RAD_MULT_CONSTANT)
#endif
#ifndef RAD_TO_DEG
#define RAD_TO_DEG(x) ((x) * RAD_TO_DEG_MULT_CONSTANT)
#endif


#endif  // BTE_MACROS_H
