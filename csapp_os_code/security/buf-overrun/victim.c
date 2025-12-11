#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    char buffer[256];
    int nread = read(0, buffer, 1024);
    printf("%d  %p\n", nread, buffer);
    fflush(stdout);
    return 0;
}
