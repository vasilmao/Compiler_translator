#include "assembly.h"
#include "parser.h"

int main(int argc, char** argv) {
    //char* s = scanString();
    FILE* input = NULL;
    open_file(&input, argv[1], "r");
    char* buffer = NULL;
    uint64_t buffer_size = 0;
    read_buffer(&buffer, &buffer_size, argv[1], input);
    Node* res = parseString(buffer);
    // Assembly(res);
    //printf("%lf\n", res);
    //printf("yeah!!     beee\n");
    graphDump(res);
    Assembly(res);
    return 0;
}
