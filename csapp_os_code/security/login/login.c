#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define PASSWORD "1234"

int main(int argc, char *argv[]) {
    // Check if running as root
    if (getuid() != 0) {
        printf("This program must be run as root\n");
        exit(1);
    }

    // Check if UID argument is provided
    if (argc != 2) {
        printf("Usage: %s <uid>\n", argv[0]);
        exit(1);
    }

    // Convert string UID to integer
    int uid = atoi(argv[1]);

    char password[100];
    printf("Enter password (1234): ");
    scanf("%s", password);

    // This is **NOT** my user password
    if (strcmp(password, PASSWORD) == 0) {

        // Linux uses kernel PAM for authentication
        // UNIX simply reads /etc/passwd and setuid
        if (setuid(uid) != 0) {
            perror("setuid failed");
            exit(1);
        }
        
        // Launch bash
        execl("/bin/bash", "bash", "-c", "whoami", NULL);
        perror("execl failed");
        exit(1);
    } else {
        printf("Invalid password\n");
        exit(1);
    }

    return 0;
}
