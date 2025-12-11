#include <stdio.h>
#include <unistd.h>

int main() {
    execl("/bin/id", "/bin/id", NULL);

    // This will not work: bash drops root privilege.
    // execl("/bin/bash", "bash", "-c", "id", NULL);
    perror("execl failed");
    return 1;
}
