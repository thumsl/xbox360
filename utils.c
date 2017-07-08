#include "utils.h"

int read_file(char** s, char* filename) {
	struct stat st;

    int i;

    if(stat(filename, &st) == -1)
        return 0;

    *s = malloc(st.st_size+1);
    memset (*s, 0, st.st_size+1);

    FILE* f;
    f = fopen(filename,"rb");
    fread(*s, 1, st.st_size, f);
    
    return 1;
}
