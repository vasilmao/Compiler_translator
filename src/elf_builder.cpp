#include "elf_builder.h"

const char elf_header[64] = {0x7f, 0x45, 0x4c, 0x46, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x02, 0x00, 0x3e, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x38, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

ElfHeader* CreateElfHeader() {
    ElfHeader* header = (ElfHeader*)calloc(1, sizeof(ElfHeader));
   // printf("%zu\n", sizeof(ElfHeader));
    for (int i = 0; i < 64; ++i) {
        header->values[i] = elf_header[i];
    }
    return header;
}

void WriteLittleInd64(uint64_t source, unsigned char* dest) {
    for (int i = 0; i < 8; ++i) {
        dest[i] = source & 0xFF;
        source >>= 8;
    }
}

void WriteLittleInd32(uint64_t source, unsigned char* dest) {
    for (int i = 0; i < 4; ++i) {
        dest[i] = source & 0xFF;
        source >>= 8;
    }
}

void WriteEntryPoint(ElfHeader* elf_header, uint64_t entry_point) {
    WriteLittleInd64(entry_point, elf_header->values + entry_point_index);
}

ProgramHeader* CreateProgramHeader() {
    ProgramHeader* ph = (ProgramHeader*)calloc(1, sizeof(ProgramHeader));
   // printf("%zu\n", sizeof(ProgramHeader));
    ph->seg_type[0] = 0x01;
    return ph;
}


void SetHeadersHeader(ProgramHeader* header) {
    header->flags[0] = 4; // elf_prog_header->flags = r_flag;
    // elf_prog_header->offset = {0x00};
    header->virtual_address[2] = 0x40; // elf_prog_header->virtual_address = {0x00, 0x00, 0x40};
    header->phys_address[2] = 0x40; // elf_prog_header->phys_address = {0x00, 0x00, 0x40};
    header->file_size[0] = elf_header_size;// elf_prog_header->file_size = {0xe8, 0x00};
    header->mem_size[0] = elf_header_size;// elf_prog_header->mem_size = {0xe8, 0x00};
    header->alignment[1] = 0x10;
}

void SetDataHeader(ProgramHeader* header) {
    header->flags[0] = 6;
    header->offset[1] = 0x10;
    header->virtual_address[1] = 0x10;
    header->virtual_address[2] = 0x40;
    header->phys_address[1] = 0x10;
    header->phys_address[2] = 0x40;
    header->file_size[0] = 0x15;
    header->mem_size[0] = 0x15;
    header->alignment[1] = 0x10;
}

void SetCodeHeader(ProgramHeader* header) {
    header->flags[0] = 5; // code_header->flags = re_flag;
    header->offset[1] = 0x20; // e8 + 15
    header->virtual_address[1] = 0x20; // code_header->virtual_address = {0xFD, 0x00, 0x40};
    header->virtual_address[2] = 0x40;
    header->phys_address[1] = 0x20;// code_header->phys_address = {0xFD, 0x00, 0x40};
    header->phys_address[2] = 0x40;
    // code_header->file_size =  strlen(code)
    // code_header->mem_size =   strlen(code)
    header->alignment[1] = 0x10;
}
