#include "assembly.h"
#include "parser.h"

const char* help_str = R"(
-h            - show this text
-o <filename> - specify out file name
-S            - make asm file instead of byte
-gd           - make graph dump of parser tree
)";

int main(int argc, const char* argv[]) {
    const char* out_filename     = "progr.out";
    const char* out_asm_filename = "progr.asm";
    const char* out_real_name;
    bool specific_name           = false;
    bool asm_version             = false;
    bool make_graph_dump         = false;
    int  optimize                = 0;
    int  input_index             = -1;
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            if (strncmp(argv[i], "-o", 2) == 0) {
                assert(!specific_name);
                i++;
                out_real_name = argv[i];
                specific_name = true;
                printf("specific name %s\n", out_real_name);
            } else if (strncmp(argv[i], "-S", 2) == 0) {
                asm_version = true;
            } else if (strncmp(argv[i], "-h", 2) == 0) {
                printf(help_str);
                return 0;
            } else if (strncmp(argv[i], "-gd", 3) == 0) {
                make_graph_dump = true;
            } else if (strncmp(argv[i], "-Oreg", 6) == 0) {
                optimize |= 1;
                printf("OPTIMIZEEE\n");
            } else {
                assert(input_index == -1);
                input_index = i;
                printf("in real name %s\n", argv[i]);
            }
        }
    }
    if (!specific_name) {
        if (asm_version) {
            out_real_name = out_asm_filename;
        } else {
            out_real_name = out_filename;
        }
    }
    FILE* input = NULL;
    open_file(&input, argv[input_index], "r");
    char* buffer = NULL;
    uint64_t buffer_size = 0;
    read_buffer(&buffer, &buffer_size, argv[input_index], input);
    Node* res = parseString(buffer);
    if (make_graph_dump) {
        graphDump(res);
    }
    if (asm_version) {
        Assembly(res, out_real_name, optimize);
    } else {
        Encode(res, out_real_name, optimize);
    }
    return 0;
}
