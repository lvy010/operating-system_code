#include <stdio.h>

int main() {
	for (const char *p = "First line\n"
                         "Second line\n"
                         "Third line"; *p; p++) {
        putchar(*p);
    }
}
