#include <internal/file_io.h>

extern FILE __files[];

void free(void*);

void __close_all(void) {
    FILE* file;
    FILE* old_file;

    file = __files;
    while (file != NULL) {

        if (file->mode.file_kind != 0) {
            fclose(file);
        }

        old_file = file;
        file = old_file->next_file_struct;

        if (old_file->is_dynamically_allocated) {
            free(old_file);
        } else {
            old_file->mode.file_kind = 3;

            if (file != NULL && file->is_dynamically_allocated) {
                old_file->next_file_struct = NULL;
            }
        }
    }
}

int __flush_all(void) {
    int result = 0;
    FILE* file;

    for (file = __files; file != NULL; file = file->next_file_struct) {
        if (file->mode.file_kind != 0 && fflush(file) != 0) {
            result = -1;
        }
    }

    return result;
}
