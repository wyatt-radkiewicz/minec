#include <assert.h>
#include <stdio.h>

#include "file.h"

char *file_load_as_string(AllocInterface alloc, void *allocator, const char *path) {
    FILE *file = fopen(path, "rb");
    char *str_buf = NULL;

    if (!file) {
        printf("file_load_as_string error: Can't find the file %s\n", path);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t str_len = ftell(file);
    fseek(file, 0, SEEK_SET);

    str_buf = alloc(allocator, str_len + 1);
    assert(str_buf && "Out of memory!");
    if (fread(str_buf, 1, str_len, file) != str_len) {
        printf("file_load_as_string error: Couldn't read whole file.\n");
        str_buf[0] = '\0';
    }

    return str_buf;
}
