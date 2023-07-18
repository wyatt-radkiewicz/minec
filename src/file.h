#ifndef _FILE_H
#define _FILE_H
#include "mem.h"

char *file_load_as_string(AllocInterface alloc, void *allocator, const char *path);

#endif
