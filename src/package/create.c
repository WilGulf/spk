#define _GNU_SOURCE

#include "spk_format.h"
#include "util.h"
#include "../tar/tar.h"

#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

int create_spk(const char *dirpath) {
    char path[PATH_MAX] = "";
    FILE *spk = 0;

    struct metadata metadata;
    memset(&metadata, 0, sizeof(metadata));

    snprintf(path, sizeof(path), "%s/metadata", dirpath);
    FILE *metafile = fopen(path, "r");
    if (!metafile) {
        goto out;
    }

    char line[PATH_MAX] = "";
    while (fgets(line, sizeof(line), metafile)) {
        if (!strncmp(line, "name=", 5)) {
            snprintf(metadata.name, sizeof(metadata.name), "%s", line + 5);
            clean_string(metadata.name);
            if (!metadata.name) {

            }
        }
        if (!strncmp(line, "version=", 8)) {
            snprintf(metadata.version, sizeof(metadata.version), "%s", line + 8);
            clean_string(metadata.version);
            if (!metadata.version) {
                
            }
        }
    }

    fclose(metafile);

    snprintf(path, sizeof(path), "%s-%s.spk", metadata.name, metadata.version);
    spk = fopen(path, "wb+");
    if (!spk) {
        goto out;
    }

    
    struct spk_header header;
    memset(&header, 0, sizeof(header));
    header.magic = SPK_MAGIC;
    header.metadata_offset = sizeof(struct spk_header);
    header.metadata_size = sizeof(struct metadata);
    header.map_offset = header.metadata_offset + header.metadata_size;

    fseek(spk, header.metadata_offset, SEEK_SET);
    fwrite(&metadata, 1, sizeof(metadata), spk);

    snprintf(path, sizeof(path), "%s/destinations", dirpath);
    FILE *destinations = fopen(path, "r");
    if (destinations) {
        fseek(spk, header.map_offset, SEEK_SET);
        header.map_size = copy_file(destinations, spk, UINT64_MAX);
        fclose(destinations);
    }

    header.data_offset = header.map_offset + header.map_size;
    fseek(spk, header.data_offset, SEEK_SET);

    if (mkdir("/tmp/spk", 0755) < 0 && errno != EEXIST) {
        goto out;
    }

    snprintf(path, sizeof(path), "%s/files", dirpath);
    write_archive("/tmp/spk/files.tar.zst", path);

    FILE *tar = fopen("/tmp/spk/files.tar.zst", "rb");
    
    if (tar) {
        header.data_size = copy_file(tar, spk, UINT64_MAX);
        fclose(tar);
    }

    fseek(spk, 0, SEEK_SET);
    fwrite(&header, 1, sizeof(header), spk);

out:
    if (spk)
        fclose(spk);

    remove("/tmp/spk/files.tar.zst");

    return 0;
}