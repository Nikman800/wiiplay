#ifndef MSL_FILE_IO_H
#define MSL_FILE_IO_H
#include <types.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long __file_handle;

typedef struct _file_modes {
    unsigned int open_mode : 2;
    unsigned int io_mode : 3;
    unsigned int buffer_mode : 2;
    unsigned int file_kind : 3;
    unsigned int file_orientation : 2;
    unsigned int binary_io : 1;
} file_modes;

typedef struct _file_states {
    unsigned int io_state : 3;
    unsigned int free_buffer : 1;
    unsigned char eof;
    unsigned char error;
} file_states;

typedef int (*__pos_proc)(__file_handle file, unsigned long* position, int mode,
                          void* idle_proc);
typedef int (*__io_proc)(__file_handle file, unsigned char* buff, unsigned long* count,
                         void* idle_proc);
typedef int (*__close_proc)(__file_handle file);
typedef void (*__idle_proc)(void);

typedef struct FILE {
    __file_handle handle;                   // at 0x0
    file_modes mode;                        // at 0x4
    file_states state;                      // at 0x8
    unsigned char is_dynamically_allocated; // at 0xC
    unsigned char char_buffer;              // at 0xD
    unsigned char char_buffer_overflow;     // at 0xE
    unsigned char ungetc_buffer[2];         // at 0xF
    unsigned short ungetwc_buffer[2];       // at 0x12
    unsigned long position;                 // at 0x18
    unsigned char* buffer;                  // at 0x1C
    unsigned long buffer_size;              // at 0x20
    unsigned char* buffer_ptr;              // at 0x24
    unsigned long buffer_len;               // at 0x28
    unsigned long buffer_alignment;         // at 0x2C
    unsigned long saved_buffer_len;         // at 0x30
    unsigned long buffer_pos;               // at 0x34
    __pos_proc position_proc;               // at 0x38
    __io_proc read_proc;                    // at 0x3C
    __io_proc write_proc;                   // at 0x40
    __close_proc close_proc;                // at 0x44
    __idle_proc idle_proc;                  // at 0x48
    struct FILE* next_file_struct;          // at 0x4C
} FILE;

int fclose(FILE*);
int fflush(FILE*);

#ifdef __cplusplus
}
#endif
#endif
