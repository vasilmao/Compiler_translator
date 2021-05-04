#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <cstdint>
#include <cstring>

uint64_t  get_file_size  (const char* filename);
void open_file           (FILE**      file,           const char* filename,    const char* mode);
void read_buffer         (char**      buffer,         uint64_t*   buffer_size, const char* input_filename, FILE*     input);
