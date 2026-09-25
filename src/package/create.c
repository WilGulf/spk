#define _POSIX_C_SOURCE 200809L

#include "spk_format.h"
#include "util.h"

#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <archive.h>
#include <archive_entry.h>

void write_archive(const char *outname, const char *dirpath) {
    struct archive *a_write;
    struct archive *a_read;
    struct archive_entry *entry = NULL;

    int res = 0;
    
    a_write = archive_write_new();
    archive_write_add_filter_zstd(a_write);
    archive_write_set_format_ustar(a_write); 
    archive_write_open_filename(a_write, outname);

    a_read = archive_read_disk_new();
    archive_read_disk_set_standard_lookup(a_read);
    archive_read_disk_open(a_read, dirpath);

    while (1) {
        entry = archive_entry_new();
        res = archive_read_next_header2(a_read, entry);
        if (res == ARCHIVE_EOF) {
            archive_entry_free(entry);
            break;
        }

        archive_read_disk_descend(a_read);
        
        archive_write_header(a_write, entry);

        if (archive_entry_size(entry) > 0) {
            const void *buffer;
            size_t bytes_read;
            int64_t offset;
            while (archive_read_data_block(a_read, &buffer, &bytes_read, &offset) == ARCHIVE_OK) {
                archive_write_data(a_write, buffer, bytes_read);
            }
        }

        archive_entry_free(entry);
    }

    archive_read_close(a_read);
    archive_read_free(a_read);

    archive_write_close(a_write);
    archive_write_free(a_write);
}

int create_spk(const char *dirpath) {
    char path[256] = "";

    struct metadata metadata;
    memset(&metadata, 0, sizeof(metadata));

    snprintf(path, sizeof(path), "%s/metadata", dirpath);
    FILE *metafile = fopen(path, "r");
    if (!metafile) {
        goto out;
    }

    char line[256] = "";
    while (fgets(line, sizeof(line), metafile)) {
        if (!strncmp(line, "name=", 5)) {
            snprintf(metadata.name, sizeof(metadata.name), "%s", line + 5);
            clean_string(metadata.name);
        }
        if (!strncmp(line, "version=", 8)) {
            snprintf(metadata.version, sizeof(metadata.version), "%s", line + 8);
            clean_string(metadata.version);
        }
    }

    fclose(metafile);

    snprintf(path, sizeof(path), "%s-%s.spk", metadata.name, metadata.version);
    FILE *spk = fopen(path, "wb+");
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

    snprintf(path, sizeof(path), "%s/files/", dirpath);
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
    
    return 0;
}