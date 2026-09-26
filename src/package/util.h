#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>

size_t copy_file(FILE *src, FILE *dest, uint64_t total);
void clean_string(char *buffer);
int mkdir_p(const char *path, mode_t mode);

#endif