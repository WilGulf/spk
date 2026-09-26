#define _GNU_SOURCE

#include "spk_format.h"
#include "util.h"

#include "../status.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

size_t copy_file(FILE *src, FILE *dest, uint64_t total) {
    char buffer[64 * 1024];

    uint64_t remaining = total;
    size_t total_read = 0;

    while (remaining > 0) {
        size_t amount = remaining < sizeof(buffer) ? remaining : sizeof(buffer);
        size_t bytes = fread(buffer, 1, amount, src);

        if (bytes == 0) {
            break;
        }

        if (fwrite(buffer, 1, bytes, dest) != bytes) {
            break;
        }

        remaining -= bytes;
        total_read += bytes;
    }

    return total_read;
}

int copy_recursive(const char *srcdir, const char *destdir) {
    if (mkdir_p(destdir, 0755) < 0) {
        return ERR_MKDIR;
    }

    DIR *dir = opendir(srcdir);
    if (!dir) {
        return ERR_FOPEN;
    }

    struct dirent *entry;
    int err_code = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
            continue;
        }

        char srcpath[PATH_MAX];
        char destpath[PATH_MAX];

        snprintf(srcpath, sizeof(srcpath), "%s/%s", srcdir, entry->d_name);
        snprintf(destpath, sizeof(destpath), "%s/%s", destdir, entry->d_name);

        struct stat st;
        if (lstat(srcpath, &st) < 0) {
            err_code = ERR_STAT;
            break;
        }

        if (S_ISDIR(st.st_mode)) {
            err_code = copy_recursive(srcpath, destpath);
            if (err_code != 0) {
                break;
            }
        } else if (S_ISREG(st.st_mode)) {
            FILE *srcfile = fopen(srcpath, "rb");
            FILE *destfile = fopen(destpath, "wb");

            if (!(srcfile && destfile)) {
                if (srcfile)
                    fclose(srcfile);
                if (destfile)
                    fclose(destfile);

                err_code = ERR_FOPEN;
                break;
            }

            copy_file(srcfile, destfile, UINT64_MAX);
            fclose(srcfile);
            fclose(destfile);
            chmod(destpath, st.st_mode & 07777);
        }
    }

    closedir(dir);
    return err_code;
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

bool check_spk(FILE *file) {
    struct spk_header header;
    fseek(file, 0, SEEK_SET);
    if (fread(&header, 1, sizeof(header), file) != sizeof(header)) {
        return false;
    }

    return header.magic == SPK_MAGIC;
}

int mkdir_p(const char *path, mode_t mode) {
    char tmp[PATH_MAX];
    snprintf(tmp, sizeof(tmp), "%s", path);
    size_t len = strlen(tmp);

    if (len == 0) {
        return 1;
    }

    if (tmp[len - 1] == '/') {
        tmp[len - 1] = '\0';
    }

    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';

            if (mkdir(tmp, mode) < 0 && errno != EEXIST) {
                return ERR_MKDIR;
            }

            *p = '/';
        }
    }

    if (mkdir(tmp, mode) < 0 && errno != EEXIST) {
        return ERR_MKDIR;
    }

    return 0;
}

struct metadata *get_spk_metadata(FILE *spk) {
    struct spk_header header;
    fseek(spk, 0, SEEK_SET);
    if (fread(&header, 1, sizeof(header), spk) != sizeof(header)) {
        return 0;
    }

    struct metadata *metadata = malloc(sizeof(struct metadata));
    fseek(spk, header.metadata_offset, SEEK_SET);
    if (fread(metadata, 1, sizeof(struct metadata), spk) != sizeof(struct metadata)) {
        free(metadata);
        return 0;
    }

    return metadata;
}