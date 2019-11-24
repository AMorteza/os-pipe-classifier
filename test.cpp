#include <stdio.h>
int main(int argc, char** argv) {
    FILE* fp = fopen("./myFIFO", "w");
    fprintf(fp, "Hello, world!\n");
    fclose(fp);
    return 0;
}