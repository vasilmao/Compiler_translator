#ifndef ELF_BUILDER_HEADER
#define ELF_BUILDER_HEADER

#include "stdio.h"
#include "stdlib.h"
#include <stdlib.h>
#include <cstdint>
#include <string.h>

const int entry_point_index = 24;
const int entry_point_size  = 8;

struct ElfHeader {
    unsigned char values[64];
};

struct ProgramHeader {
    unsigned char seg_type[4];
    unsigned char flags[4];
    unsigned char offset[8];
    unsigned char virtual_address[8];
    unsigned char phys_address[8];
    unsigned char file_size[8];
    unsigned char mem_size[8];
    unsigned char alignment[8];
};

ElfHeader* CreateElfHeader();
void WriteLittleInd64(uint64_t source, unsigned char* dest);
void WriteLittleInd32(uint64_t source, unsigned char* dest);
void WriteEntryPoint(ElfHeader* elf_header, uint64_t entry_point);
ProgramHeader* CreateProgramHeader();

#endif
