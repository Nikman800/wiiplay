#include <internal/file_io.h>
#include <internal/mem.h>
#include <internal/wchar_io.h>

int _fseek(FILE*, long, int);
void __prep_buffer(FILE*);
int __flush_buffer(FILE*, unsigned long*);
void __stdio_atexit(void);

enum {
    __neutral,
    __writing,
    __reading
};

enum {
    __read = 1,
    __write = 2,
    __append = 4
};

enum {
    __closed_file,
    __disk_file,
    __console_file
};

#define _IONBF 0
#define _IOLBF 1
#define _IOFBF 2

#define set_error(file)          \
    do {                         \
        (file)->state.error = 1; \
        (file)->buffer_len = 0;  \
    } while (0)

size_t __fwrite(const void* buffer, size_t size, size_t count, FILE* stream) {
    unsigned char* write_ptr;
    size_t num_bytes, bytes_to_go, bytes_written;
    int always_buffer;
    int ioresult;

    if (fwide(stream, 0) == 0)
        fwide(stream, -1);

    bytes_to_go = size * count;

    if (!bytes_to_go || stream->state.error || stream->mode.file_kind == __closed_file)
        return 0;

    if (stream->mode.file_kind == __console_file)
        __stdio_atexit();

    always_buffer = !stream->mode.binary_io || stream->mode.buffer_mode == _IOFBF ||
                    stream->mode.buffer_mode == _IOLBF;

    if (stream->state.io_state == __neutral) {
        if (stream->mode.io_mode & __write) {
            if (stream->mode.io_mode & __append) {
                if (_fseek(stream, 0, 2))
                    return 0;
            }

            stream->state.io_state = __writing;
            __prep_buffer(stream);
        }
    }

    if (stream->state.io_state != __writing) {
        set_error(stream);
        return 0;
    }

    write_ptr = (unsigned char*)buffer;
    bytes_written = 0;

    if (bytes_to_go && (stream->buffer_ptr != stream->buffer || always_buffer)) {
        stream->buffer_len = stream->buffer_size - (stream->buffer_ptr - stream->buffer);

        do {
            unsigned char* newline = NULL;

            num_bytes = stream->buffer_len;

            if (num_bytes > bytes_to_go)
                num_bytes = bytes_to_go;

            if (stream->mode.buffer_mode == _IOLBF && num_bytes)
                if ((newline = (unsigned char*)__memrchr(write_ptr, '\n', num_bytes)) != NULL)
                    num_bytes = newline + 1 - write_ptr;

            if (num_bytes) {
                memcpy(stream->buffer_ptr, write_ptr, num_bytes);

                write_ptr += num_bytes;
                bytes_to_go -= num_bytes;

                stream->buffer_ptr += num_bytes;
                stream->buffer_len -= num_bytes;
            }

            if (!stream->buffer_len || newline != NULL || stream->mode.buffer_mode == _IONBF) {
                ioresult = __flush_buffer(stream, NULL);

                if (ioresult) {
                    set_error(stream);
                    bytes_to_go = 0;
                    break;
                }
            }

            bytes_written += num_bytes;
        } while (bytes_to_go && always_buffer);
    }

    if (bytes_to_go && !always_buffer) {
        unsigned char* save_buffer = stream->buffer;
        size_t save_size = stream->buffer_size;

        stream->buffer = write_ptr;
        stream->buffer_size = bytes_to_go;
        stream->buffer_ptr = write_ptr + bytes_to_go;

        if (__flush_buffer(stream, &num_bytes))
            set_error(stream);
        else
            bytes_written += num_bytes;

        stream->buffer = save_buffer;
        stream->buffer_size = save_size;

        __prep_buffer(stream);

        stream->buffer_len = 0;
    }

    if (stream->mode.buffer_mode != _IOFBF)
        stream->buffer_len = 0;

    return bytes_written / size;
}
