#define _GNU_SOURCE

#include "spk_format.h"
#include "package.h"
#include "util.h"
#include "../tar/tar.h"

#include "../status.h"

#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>

char *extract_spk(const char *spkpath, int *err_code) {
    // FILES OPENED
    FILE *spk = NULL;
    FILE *dest_out = NULL;
    FILE *files_out = NULL;

    printf("extracting %s\n", spkpath);

    char *output = 0;
    *err_code = 0;

    char path[PATH_MAX] = "";

    spk = fopen(spkpath, "rb");
    if (!spk) {
        *err_code = ERR_FOPEN;
        goto err_out;
    }

    if (!check_spk(spk)) {
        *err_code = ERR_MAGIC;
        goto err_out;
    }

    struct spk_header header;
    memset(&header, 0, sizeof(header));
    fseek(spk, 0, SEEK_SET);
    fread(&header, 1, sizeof(struct spk_header), spk);

    struct metadata metadata;
    memset(&metadata, 0, sizeof(metadata));
    fseek(spk, header.metadata_offset, SEEK_SET);
    fread(&metadata, 1, sizeof(struct metadata), spk);

    output = malloc(PATH_MAX);
    if (!output) {
        *err_code = ERR_MALLOC;
        goto err_out;
    }
    
    printf("Setting up work directories\n");
    snprintf(output, PATH_MAX, "/tmp/spk/%s", metadata.name);
    if (mkdir("/tmp/spk", 0755) < 0 && errno != EEXIST) {
        *err_code = ERR_MKDIR;
        goto err_out;
    }
    if (mkdir(output, 0755) < 0 && errno != EEXIST) {
        *err_code = ERR_MKDIR;
        goto err_out;
    }

    snprintf(path, sizeof(path), "%s/destinations", output);
    dest_out = fopen(path, "wb");
    if (!dest_out) {
        *err_code = ERR_FOPEN;
        goto err_out;
    }
    
    fseek(spk, header.map_offset, SEEK_SET);
    copy_file(spk, dest_out, header.map_size);
    fclose(dest_out);
    dest_out = NULL;

    snprintf(path, sizeof(path), "%s/files.tar.zst", output);
    files_out = fopen(path, "wb");
    if (!files_out) {
        *err_code = ERR_FOPEN;
        goto err_out;
    }

    fseek(spk, header.data_offset, SEEK_SET);
    copy_file(spk, files_out, header.data_size);
    fclose(files_out);
    files_out = NULL;

    char path_out[PATH_MAX];
    snprintf(path_out, sizeof(path_out), "%s/files", output);
    snprintf(path, sizeof(path), "%s/files.tar.zst", output);
    if (mkdir(path_out, 0755) < 0 && errno != EEXIST) {
        *err_code = ERR_MKDIR;
        goto err_out;
    }
    extract_archive(path, path_out);

    *err_code = 0;
    goto out;

err_out:
    if (output)
        free(output);

    output = 0;

out:
    if (spk)
        fclose(spk);
    if (dest_out)
        fclose(dest_out);
    if (files_out)
        fclose(files_out);

    return output;
}