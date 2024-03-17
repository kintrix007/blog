#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    char buf[32];

    if (argc < 2) {
        printf("Expected argument.\n");
        return 1;
    }

    strcpy(buf, argv[1]);

    return 0;
}

void unused_function(void) {
    printf("Pwned.\n");
}
