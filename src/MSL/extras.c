#include <ctype.h>
#include <string.h>

int stricmp(const char* left, const char* right) {
    signed char left_character;
    signed char right_character;

    for (;;) {
        left_character = (signed char)tolower(*left++);
        right_character = (signed char)tolower(*right++);

        if (left_character < right_character)
            return -1;

        if (left_character > right_character)
            return 1;

        if (left_character == '\0')
            return 0;
    }
}
