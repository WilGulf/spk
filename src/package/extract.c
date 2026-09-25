#include "spk_format.h"
#include "util.h"

#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

int extract_spk(const char *spkpath) {
    char path[256] = "";

    FILE *spk = fopen(spkpath, "rb");
    if (!spk) {
        goto out;
    }

    struct spk_header header;
    fread(&header, 1, sizeof(struct spk_header), spk);

    struct metadata metadata;
    fseek(spk, header.metadata_offset, SEEK_SET);
    fread(&metadata, 1, sizeof(struct metadata), spk);

    char workdir[256];
    snprintf(workdir, sizeof(workdir), "/tmp/spk/%s", metadata.name);
    if (mkdir("/tmp/spk", 0755) < 0 && errno != EEXIST) {
        goto out;
    }
    if (mkdir(workdir, 0755) < 0 && errno != EEXIST) {
        goto out;
    }

    snprintf(path, sizeof(path), "%s/destinations", workdir);
    FILE *dest_out = fopen(path, "wb");
    if (!dest_out) {
        goto out;
    }
    
    fseek(spk, header.map_offset, SEEK_SET);
    copy_file(spk, dest_out, header.map_size);
    fclose(dest_out);

    snprintf(path, sizeof(path), "%s/files.tar.zst", workdir);
    FILE *files_out = fopen(path, "wb");
    if (!files_out) {
        goto out;
    }
    fseek(spk, header.data_offset, SEEK_SET);
    copy_file(spk, files_out, header.data_size);
    fclose(files_out);

out:
    return 0;
}