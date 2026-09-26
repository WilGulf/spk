#include <stdio.h>
#include <stdbool.h>

#include "spk_format.h"

#ifndef PACKAGE_H
#define PACKAGE_H

int create_spk(const char *dirpath);
char *extract_spk(const char *spkpath, int *err_code);
bool check_spk(FILE *file);
struct metadata *get_spk_metadata(FILE *spk);

#endif