#ifndef ELF_BUILDER_HEADER
#define ELF_BUILDER_HEADER

#include "stdio.h"
#include "stdlib.h"
#include <stdlib.h>
#include <cstdint>
#include <string.h>

const int entry_point_index = 24;
const int entry_point_size  = 8;
typedef unsigned char byte;
const byte elf_header_size = 0xE8;

struct ElfHeader {
    byte values[64];
};

struct ProgramHeader {
    byte seg_type[4];
    byte flags[4];
    byte offset[8];
    byte virtual_address[8];
    byte phys_address[8];
    byte file_size[8];
    byte mem_size[8];
    byte alignment[8];
};

ElfHeader* CreateElfHeader();
void WriteLittleInd64(uint64_t source, byte* dest);
void WriteLittleInd32(uint64_t source, byte* dest);
void WriteEntryPoint(ElfHeader* elf_header, uint64_t entry_point);
ProgramHeader* CreateProgramHeader();

void SetHeadersHeader(ProgramHeader* header);
void SetDataHeader(ProgramHeader* header);
void SetCodeHeader(ProgramHeader* header);

#endif
