#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

size_t copy_file(FILE *src, FILE *dest, uint64_t total) {
    char buffer[64 * 1024];

    size_t remaining = total;
    size_t total_read = 0;

    int c;
    while ((c = fgetc(src)) != EOF && remaining > 0) {
        fputc(c, dest);
        remaining--;
        total_read++;
    }

    return total_read;
}

void clean_string(char *buffer) {
    int i = 0;
    while (buffer[i]) {
        if (buffer[i] == '"' || buffer[i] == '\n' || buffer[i] == '=') {
            memmove(&buffer[i], &buffer[i + 1], strlen(&buffer[i + 1]) + 1);
            i--;
        }
        i++;
    }
}