#include <internal/wchar_io.h>

enum {
    __unoriented,
    __wide_oriented,
    __narrow_oriented,
    __other_oriented
};

int fwide(FILE* file, int mode) {
    if (file == NULL || file->mode.file_kind == 0) {
        return 0;
    }

    switch (file->mode.file_orientation) {
        case __unoriented:
            if (mode > 0) {
                file->mode.file_orientation = __narrow_oriented;
            } else if (mode < 0) {
                file->mode.file_orientation = __wide_oriented;
            }
            break;

        case __narrow_oriented:
            mode = 1;
            break;

        case __wide_oriented:
            mode = -1;
            break;
        }

    return mode;
}
