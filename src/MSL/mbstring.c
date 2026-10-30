#include <ctype.h>
#include <string.h>

int mbtowc(wchar_t* pwc, const char* s, size_t n) {
    return _current_locale.cmap->decode_mb(pwc, s, n);
}

int __mbtowc_noconv(wchar_t* pwc, const char* s, size_t n) {
    if (s == NULL) {
        return 0;
    }

    if (n == 0) {
        return -1;
    }

    if (pwc != NULL) {
        *pwc = (unsigned char)*s;
    }

    if (*s == '\0') {
        return 0;
    }

    return 1;
}

int __wctomb_noconv(char* s, wchar_t wchar) {
    if (s == NULL) {
        return 0;
    }

    *s = wchar;
    return 1;
}

size_t mbstowcs(wchar_t* pwcs, const char* s, size_t n) {
    int result;
    const char* source;
    int count;
    size_t source_len;

    source_len = strlen(s);

    if (pwcs != NULL) {
        source = s;

        for (count = 0; count < n; count++) {
            if (*source) {
                result = _current_locale.cmap->decode_mb(pwcs++, source, source_len);

                if (result > 0) {
                    source += result;
                    source_len -= result;
                } else {
                    return -1;
                }
            } else {
                *pwcs = 0;
                break;
            }
        }
    } else {
        count = 0;
    }

    return count;
}

size_t wcstombs(char* s, const wchar_t* pwcs, size_t n) {
    int result;
    int chars_written = 0;
    char temp[8];
    const wchar_t* source;

    if (!s || !pwcs) {
        return 0;
    }

    source = pwcs;

    while (chars_written <= n) {
        if (!*source) {
            *(s + chars_written) = '\0';
            break;
        } else {
            result = _current_locale.cmap->encode_wc(temp, *source++);

            if ((chars_written + result) <= n) {
                strncpy(s + chars_written, temp, result);
                chars_written += result;
            } else {
                break;
            }
        }
    }

    return chars_written;
}
