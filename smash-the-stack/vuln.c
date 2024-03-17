#include <stdio.h>
#include <string.h>

#define BUF_SIZE 32

int main(int argc, char **argv) {
    char buf[BUF_SIZE];

    if (argc < 2) {
        printf("Usage: vuln <input>\n");
        return 1;
    }

    strcpy(buf, argv[1]);

    return 0;
}

void unused_function(void) {
    printf("Pwned.\n");
}
