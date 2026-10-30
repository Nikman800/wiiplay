#include <errno.h>
#include <internal/file_io.h>

enum {
    __neutral,
    __writing,
    __reading,
    __rereading
};

enum {
    __read = 1,
    __write = 2,
    __read_write = 3,
    __append = 4
};

enum {
    __closed_file,
    __disk_file,
    __console_file
};

#define EFPOS 0x28

#define set_error(file)          \
    do {                         \
        (file)->state.error = 1; \
        (file)->buffer_len = 0;  \
    } while (0)

int __flush_buffer(FILE* file, unsigned long* bytes_flushed);

int _ftell(FILE* file) {
    int charsInUndoBuffer = 0;
    int position;
    unsigned char tmp_kind = file->mode.file_kind;

    if (!(tmp_kind == __disk_file || tmp_kind == __console_file) || file->state.error) {
        errno = EFPOS;
        return -1;
    }

    if (file->state.io_state == __neutral)
        return file->position;

    position = file->buffer_pos + (file->buffer_ptr - file->buffer);

    if (file->state.io_state >= __rereading) {
        charsInUndoBuffer = file->state.io_state - __reading;
        position -= charsInUndoBuffer;
    }

    if (!file->mode.binary_io) {
        int n = file->buffer_ptr - file->buffer - charsInUndoBuffer;
        unsigned char* p = (unsigned char*)file->buffer;

        while (n--) {
            if (*p++ == '\n')
                position++;
        }
    }

    return position;
}

long ftell(FILE* file) {
    return _ftell(file);
}

int _fseek(FILE* file, unsigned long offset, int file_mode) {
    if (file->mode.file_kind != __disk_file || file->state.error) {
        errno = EFPOS;
        return -1;
    }

    if (file->state.io_state == __writing) {
        if (__flush_buffer(file, NULL)) {
            set_error(file);
            errno = EFPOS;
            return -1;
        }
    }

    if (file_mode == 1) {
        file_mode = 0;
        offset += _ftell(file);
    }

    if (file_mode != 2 && file->mode.io_mode != __read_write &&
        (file->state.io_state == __reading || file->state.io_state == __rereading)) {
        if (offset >= file->position || offset < file->buffer_pos) {
            file->state.io_state = __neutral;
        } else {
            file->buffer_ptr = file->buffer + (offset - file->buffer_pos);
            file->buffer_len = file->position - offset;
            file->state.io_state = __reading;
        }
    } else {
        file->state.io_state = __neutral;
    }

    if (file->state.io_state == __neutral) {
        if (file->position_proc != NULL &&
            (*file->position_proc)(file->handle, &offset, file_mode, file->idle_proc)) {
            set_error(file);
            errno = EFPOS;
            return -1;
        } else {
            file->state.eof = 0;
            file->position = offset;
            file->buffer_len = 0;
        }
    }

    return 0;
}
