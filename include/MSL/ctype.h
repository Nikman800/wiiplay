#ifndef MSL_CTYPE_H
#define MSL_CTYPE_H
#include <types.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct __CMap {
    char name[8];
    const u16* ctype_map;     // at 0x8
    const u8* to_upper_table; // at 0xC
    const u8* to_lower_table; // at 0x10
    const u16* wctype_map;    // at 0x14
    const u16* wupper_map;    // at 0x18
    const u16* wlower_map;    // at 0x1C
    int (*decode_mb)(u16* pwc, const char* s, unsigned long n); // at 0x20
    int (*encode_wc)(char* s, u16 wchar);                       // at 0x24
} __CMap;

#define __ctype_alpha 0x0001
#define __ctype_digit 0x0008
#define __ctype_space 0x0100
#define __ctype_upper 0x0200
#define __ctype_xdigit 0x0400

typedef struct __Locale {
    char UNK_0x0[0x38];
    struct __CMap* cmap; // at 0x38
    char UNK_0x3C[0x44 - 0x3C];
} __Locale;

extern __Locale _current_locale;

inline int isalpha(int x) {
    return (x < 0 || x >= 256)
               ? 0
               : (int)((&_current_locale)->cmap->ctype_map[x] & __ctype_alpha);
}

inline int isdigit(int x) {
    return (x < 0 || x >= 256)
               ? 0
               : (int)((&_current_locale)->cmap->ctype_map[x] & __ctype_digit);
}

inline int isspace(int x) {
    return (x < 0 || x >= 256)
               ? 0
               : (int)((&_current_locale)->cmap->ctype_map[x] & __ctype_space);
}

inline int isxdigit(int x) {
    return (x < 0 || x >= 256)
               ? 0
               : (int)((&_current_locale)->cmap->ctype_map[x] & __ctype_xdigit);
}

inline int isupper(int x) {
    return (x < 0 || x >= 256)
               ? 0
               : (int)((&_current_locale)->cmap->ctype_map[x] & __ctype_upper);
}

inline int tolower(int x) {
    return (x < 0 || x >= 256)
               ? x
               : (int)(&_current_locale)->cmap->to_lower_table[x];
}

inline int toupper(int x) {
    return (x < 0 || x >= 256)
               ? x
               : (int)(&_current_locale)->cmap->to_upper_table[x];
}

#ifdef __cplusplus
}
#endif
#endif
