#include <types.h>
#include <MSL/ctype.h>
#include <MSL/locale.h>
#include <internal/float.h>
#include <internal/mem.h>

#include <limits.h>

typedef struct decimal {
    char sign;
    char unused;
    short exp;
    struct {
        unsigned char length;
        unsigned char text[36];
        unsigned char unused;
    } sig;
} decimal;

extern struct lconv __lconv;

double __dec2num(const decimal* d);
double nan(const char* arg);

enum scan_states {
    start = 0x0001,
    sig_start = 0x0002,
    leading_sig_zeroes = 0x0004,
    int_digit_loop = 0x0008,
    frac_start = 0x0010,
    frac_digit_loop = 0x0020,
    sig_end = 0x0040,
    exp_start = 0x0080,
    leading_exp_digit = 0x0100,
    leading_exp_zeroes = 0x0200,
    exp_digit_loop = 0x0400,
    finished = 0x0800,
    failure = 0x1000,
    nan_state = 0x2000,
    infin_state = 0x4000,
    hex_state = 0x8000
};

enum hex_scan_states {
    not_hex = 0x0000,
    hex_start = 0x0001,
    hex_leading_sig_zeroes = 0x0002,
    hex_int_digit_loop = 0x0004,
    hex_frac_digit_loop = 0x0008,
    hex_sig_end = 0x0010,
    hex_exp_start = 0x0020,
    hex_leading_exp_digit = 0x0040,
    hex_leading_exp_zeroes = 0x0080,
    hex_exp_digit_loop = 0x0100
};

#define final_state(scan_state) (scan_state & (finished | failure))
#define success(scan_state)                                                                        \
    (scan_state & (leading_sig_zeroes | int_digit_loop | frac_digit_loop | leading_exp_zeroes |   \
                   exp_digit_loop | finished))
#define hex_success(count, scan_state)                                                             \
    (count - 1 > 2 &&                                                                              \
     scan_state & (hex_leading_sig_zeroes | hex_int_digit_loop | hex_frac_digit_loop |            \
                   hex_leading_exp_zeroes | hex_exp_digit_loop))

#define fetch() (count++, (*ReadProc)(ReadProcArg, 0, 0))
#define unfetch(c) (*ReadProc)(ReadProcArg, c, 1)

#define TARGET_FLOAT_BITS 64
#define TARGET_FLOAT_MANT_BITS 52
#define TARGET_FLOAT_EXP_BITS 11
#define TARGET_FLOAT_MAX_EXP 1024
#define SIGDIGLEN 20
#define LDBL_MANT_DIG 53

long double __strtold(int max_width, int (*ReadProc)(void*, int, int), void* ReadProcArg,
                      int* chars_scanned, int* overflow) {
    int scan_state = start;
    int hex_scan_state = not_hex;
    int count = 0;
    int spaces = 0;
    int c;
    decimal d = {0, 0, 0, {0, ""}};
    int sig_negative = 0;
    int exp_negative = 0;
    long exp_value = 0;
    int exp_adjust = 0;
    long double result;
    int sign_detected = 0;
    int radix_marker = *(unsigned char*)__lconv.decimal_point;

    unsigned char* chptr;
    unsigned char mantissa[8];
    unsigned long exponent = 0;
    int ui;
    unsigned char uch, uch1;
    int NibbleIndex;
    int expsign = 0;
    unsigned intdigits = 0;
    unsigned mantissa_digits;

    *overflow = 0;

    c = fetch();

    while (count <= max_width && c != -1 && !final_state(scan_state)) {
        switch (scan_state) {
        case start:
            if (isspace(c)) {
                c = fetch();
                count--;
                spaces++;
                break;
            }

            switch (toupper(c)) {
            case '-':
                sig_negative = 1;

            case '+':
                c = fetch();
                sign_detected = 1;
                break;

            case 'I':
                c = fetch();
                scan_state = infin_state;
                break;

            case 'N':
                c = fetch();
                scan_state = nan_state;
                break;

            default:
                scan_state = sig_start;
                break;
            }
            break;

        case infin_state: {
            int i = 1;
            char model[] = "INFINITY";

            while (i < 8 && toupper(c) == model[i]) {
                i++;
                c = fetch();
            }

            if (i == 3 || i == 8) {
                result = sig_negative ? (float)-(double)*(float*)__float_huge : *(float*)__float_huge;
                *chars_scanned = spaces + i + sign_detected;
                return result;
            } else {
                scan_state = failure;
            }
            break;
        }

        case nan_state: {
            int i = 1, j = 0;
            char model[] = "NAN(";
            char nan_arg[32] = "";

            while (i < 4 && toupper(c) == model[i]) {
                i++;
                c = fetch();
            }

            if (i == 3 || i == 4) {
                if (i == 4) {
                    while (j < 32 && (isdigit(c) || isalpha(c) || c == radix_marker)) {
                        nan_arg[j++] = c;
                        c = fetch();
                    }

                    if (c != ')') {
                        scan_state = failure;
                        break;
                    } else {
                        j++;
                    }
                }
                nan_arg[j] = '\0';

                if (sig_negative) {
                    result = -nan(nan_arg);
                } else {
                    result = nan(nan_arg);
                }

                *chars_scanned = spaces + i + j + sign_detected;
                return result;
            } else {
                scan_state = failure;
            }
            break;
        }

        case sig_start:
            if (c == radix_marker) {
                scan_state = frac_start;
                c = fetch();
                break;
            }

            if (!isdigit(c)) {
                scan_state = failure;
                break;
            }

            if (c == '0') {
                c = fetch();
                if (toupper(c) == 'X') {
                    scan_state = hex_state;
                    hex_scan_state = hex_start;
                } else {
                    scan_state = leading_sig_zeroes;
                }
                break;
            }

            scan_state = int_digit_loop;
            break;

        case leading_sig_zeroes:
            if (c == '0') {
                c = fetch();
                break;
            }

            scan_state = int_digit_loop;
            break;

        case int_digit_loop:
            if (!isdigit(c)) {
                if (c == radix_marker) {
                    scan_state = frac_digit_loop;
                    c = fetch();
                } else {
                    scan_state = sig_end;
                }
                break;
            }

            if (d.sig.length < SIGDIGLEN) {
                d.sig.text[d.sig.length++] = c;
            } else {
                exp_adjust++;
            }

            c = fetch();
            break;

        case frac_start:
            if (!isdigit(c)) {
                scan_state = failure;
                break;
            }

            scan_state = frac_digit_loop;
            break;

        case frac_digit_loop:
            if (!isdigit(c)) {
                scan_state = sig_end;
                break;
            }

            if (d.sig.length < SIGDIGLEN) {
                if (c != '0' || d.sig.length) {
                    d.sig.text[d.sig.length++] = c;
                }
                exp_adjust--;
            }

            c = fetch();
            break;

        case sig_end:
            if (toupper(c) == 'E') {
                scan_state = exp_start;
                c = fetch();
                break;
            }

            scan_state = finished;
            break;

        case exp_start:
            if (c == '+') {
                c = fetch();
            } else if (c == '-') {
                c = fetch();
                exp_negative = 1;
            }

            scan_state = leading_exp_digit;
            break;

        case leading_exp_digit:
            if (!isdigit(c)) {
                scan_state = failure;
                break;
            }

            if (c == '0') {
                scan_state = leading_exp_zeroes;
                c = fetch();
                break;
            }

            scan_state = exp_digit_loop;
            break;

        case leading_exp_zeroes:
            if (c == '0') {
                c = fetch();
                break;
            }

            scan_state = exp_digit_loop;
            break;

        case exp_digit_loop:
            if (!isdigit(c)) {
                scan_state = finished;
                break;
            }

            exp_value = exp_value * 10 + (c - '0');
            if (exp_value > SHRT_MAX) {
                *overflow = 1;
            }

            c = fetch();
            break;

        case hex_state:
            switch (hex_scan_state) {
            case hex_start:
                memset(mantissa, 0, sizeof(mantissa));
                chptr = mantissa;
                intdigits = 0;
                mantissa_digits = (LDBL_MANT_DIG + 3) / 4;
                NibbleIndex = 0;
                hex_scan_state = hex_leading_sig_zeroes;
                c = fetch();
                break;

            case hex_leading_sig_zeroes:
                if (c == '0') {
                    c = fetch();
                    break;
                }

                hex_scan_state = hex_int_digit_loop;
                break;

            case hex_int_digit_loop:
                if (!isxdigit(c)) {
                    if (c == radix_marker) {
                        hex_scan_state = hex_frac_digit_loop;
                        c = fetch();
                    } else {
                        hex_scan_state = hex_sig_end;
                    }
                    break;
                }

                if (intdigits < mantissa_digits) {
                    intdigits++;
                    uch = chptr[NibbleIndex / 2];
                    ui = toupper(c);

                    if (ui >= 'A') {
                        ui = ui - 'A' + 10;
                    } else {
                        ui -= '0';
                    }

                    uch1 = ui;

                    if (NibbleIndex % 2) {
                        uch |= uch1;
                    } else {
                        uch |= (unsigned char)(uch1 << 4);
                    }

                    chptr[NibbleIndex++ / 2] = uch;
                    c = fetch();
                } else {
                    c = fetch();
                }
                break;

            case hex_frac_digit_loop:
                if (!isxdigit(c)) {
                    hex_scan_state = hex_sig_end;
                    break;
                }

                if (intdigits < mantissa_digits) {
                    uch = chptr[NibbleIndex / 2];
                    ui = toupper(c);

                    if (ui >= 'A') {
                        ui = ui - 'A' + 10;
                    } else {
                        ui -= '0';
                    }

                    uch1 = ui;

                    if (NibbleIndex % 2) {
                        uch |= uch1;
                    } else {
                        uch |= (unsigned char)(uch1 << 4);
                    }

                    chptr[NibbleIndex++ / 2] = uch;
                    c = fetch();
                } else {
                    c = fetch();
                }
                break;

            case hex_sig_end:
                if (toupper(c) == 'P') {
                    hex_scan_state = hex_exp_start;
                    c = fetch();
                    break;
                }

                scan_state = finished;
                break;

            case hex_exp_start:
                if (c == '-') {
                    expsign = 1;
                } else if (c != '+') {
                    unfetch(c);
                    count--;
                }

                hex_scan_state = hex_leading_exp_digit;
                c = fetch();
                break;

            case hex_leading_exp_digit:
                if (!isdigit(c)) {
                    scan_state = failure;
                    break;
                }

                if (c == '0') {
                    hex_scan_state = hex_leading_exp_zeroes;
                    c = fetch();
                    break;
                }

                hex_scan_state = hex_exp_digit_loop;
                break;

            case hex_leading_exp_zeroes:
                if (c == '0') {
                    c = fetch();
                    break;
                }

                hex_scan_state = hex_exp_digit_loop;
                break;

            case hex_exp_digit_loop:
                if (!isdigit(c)) {
                    scan_state = finished;
                    break;
                }

                exponent = exponent * 10 + (c - '0');
                if (exp_value > SHRT_MAX) {
                    *overflow = 1;
                }

                c = fetch();
                break;
            }
            break;
        }
    }

    if (scan_state != hex_state ? !success(scan_state) : !hex_success(count, hex_scan_state)) {
        *chars_scanned = 0;
    } else {
        *chars_scanned = count + spaces - 1;
    }

    unfetch(c);

    if (hex_scan_state == not_hex) {
        if (exp_negative) {
            exp_value = -exp_value;
        }

        {
            int n = d.sig.length;
            unsigned char* p = &d.sig.text[n];

            while (n-- && *--p == '0') {
                exp_adjust++;
            }

            d.sig.length = n + 1;

            if (d.sig.length == 0) {
                d.sig.text[d.sig.length++] = '0';
            }
        }

        exp_value += exp_adjust;

        if (exp_value < -32768 || exp_value > SHRT_MAX) {
            *overflow = 1;
        }

        if (*overflow) {
            if (exp_negative) {
                return 0.0;
            } else if (sig_negative) {
                return -*(double*)__double_huge;
            } else {
                return *(double*)__double_huge;
            }
        }

        d.exp = exp_value;

        result = __dec2num(&d);

        if (result != 0.0 && result < 2.2250738585072014e-308) {
            *overflow = 1;
        } else if (result > 1.7976931348623157e+308) {
            *overflow = 1;
            result = *(double*)__double_huge;
        }

        if (sig_negative && success(scan_state)) {
            result = -result;
        }

        return result;
    } else {
        long double hex_result;
        unsigned char* resptr = (unsigned char*)&hex_result;
        unsigned bit;
        unsigned dst_bit;
        unsigned shift;
        int byte_index;
        unsigned char carry;

        if (expsign) {
            exponent = -exponent;
        }

        exponent += intdigits * 4;

        /* normalize: drop the leading 1 bit of the first hex digit */
        uch = mantissa[0];
        bit = 0;
        while (bit < 4 && !(uch & (0x80 >> bit))) {
            bit++;
            exponent--;
        }

        if (bit + 1) {
            carry = 0;
            for (chptr = mantissa + 7; chptr >= mantissa; chptr--) {
                uch = *chptr;
                *chptr = (uch << (bit + 1)) | carry;
                carry = uch >> (8 - (bit + 1));
            }
        }

        memset(resptr, 0, sizeof(hex_result));

        /* place the mantissa after the sign and exponent bits */
        byte_index = 1;
        for (bit = 0; bit < TARGET_FLOAT_MANT_BITS; bit += 8) {
            carry = mantissa[bit / 8];
            dst_bit = bit + 12;
            if (bit + 8 > TARGET_FLOAT_MANT_BITS) {
                carry &= 0xff << (TARGET_FLOAT_MANT_BITS - bit);
            }
            shift = dst_bit % 8;
            resptr[byte_index] |= (unsigned char)(carry >> shift);
            resptr[++byte_index] |= (unsigned char)(carry << (8 - shift));
        }

        exponent += TARGET_FLOAT_MAX_EXP - 2;

        if (exponent & ~((1 << TARGET_FLOAT_EXP_BITS) - 1)) {
            *overflow = 1;
            return 0.0;
        }

        exponent <<= 32 - TARGET_FLOAT_EXP_BITS;
        ((unsigned char*)&hex_result)[0] |= exponent >> 25;
        ((unsigned char*)&hex_result)[1] |= exponent >> 17;

        if (sig_negative) {
            ((unsigned char*)&hex_result)[0] |= 0x80;
        }

        return hex_result;
    }
}
