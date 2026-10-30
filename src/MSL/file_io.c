#include <internal/file_io.h>

void free(void*);
int __flush_all(void);
int __flush_buffer(FILE* file, unsigned long* bytes_flushed);
long ftell(FILE* file);

enum {
    __neutral_state,
    __writing,
    __reading,
    __updating,
    __read_write
};

enum {
    __read_mode,
    __write_mode,
    __append_mode,
    __read_write_mode
};

enum {
    __closed_file,
    __disk_file,
    __console_file
};

int fclose(FILE* file) {
    int flush_result;
    int close_result;

    if (file == NULL) {
        return -1;
    }

    if (file->mode.file_kind == 0) {
        return 0;
    }

    flush_result = fflush(file);
    close_result = (*file->close_proc)(file->handle);

    file->handle = NULL;
    file->mode.file_kind = 0;

    if (file->state.free_buffer != 0) {
        free(file->buffer);
    }

    return -(flush_result != 0 || close_result != 0);
}

int fflush(FILE* file) {
    long position;
    int result;

    if (file == NULL) {
        return __flush_all();
    }

    if (file->state.error != 0 || file->mode.file_kind == 0) {
        return -1;
    }

    if (file->mode.io_mode == 1) {
        return 0;
    }

    if (file->state.io_state >= __updating) {
        file->state.io_state = __reading;
    }

    if (file->state.io_state == __reading) {
        file->buffer_len = 0;
    }

    if (file->state.io_state != __writing) {
        file->state.io_state = __neutral_state;
        return 0;
    }

    if (file->mode.file_kind != __disk_file) {
        position = 0;
    } else {
        position = ftell(file);
    }

    result = __flush_buffer(file, NULL);

    if (result != 0) {
        file->state.error = 1;
        file->buffer_len = 0;
        return -1;
    }

    file->position = position;
    file->state.io_state = 0;
    file->buffer_len = 0;
    return 0;
}
