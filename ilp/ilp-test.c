#include <stdio.h>

#define N 100000000

int x, y;

int main() {
    register int i asm("edi");
    
    for (i = 0; i < N; i++) {
        // There're too many dependencies;
        // I didn't list them.

        asm volatile(
            // General-purpose registers
            "mov $0, %rax;"
            "mov $0, %rbx;"
            "mov $0, %rcx;"
            "mov $0, %rdx;"
            "mov $0, %rsi;"
            "mov $0, %r8;"
            "mov $0, %r9;"
            "mov $0, %r10;"
            "mov $0, %r11;"

            // SIMD registers
            "movaps %xmm0, %xmm1;"
            "movaps %xmm0, %xmm2;"
            "movaps %xmm0, %xmm3;"
            "movaps %xmm0, %xmm4;"
            "movaps %xmm0, %xmm5;"
            "movaps %xmm0, %xmm6;"
            "movaps %xmm0, %xmm7;"
            "movaps %xmm0, %xmm8;"
        );

    }
}
