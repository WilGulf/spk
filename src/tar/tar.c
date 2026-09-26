#define _GNU_SOURCE

#include "tar.h"

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

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

        const char *current_path = archive_entry_pathname(entry);
        const char *rel_path = current_path + strlen(dirpath);
        while (*rel_path == '/') {
            rel_path++;
        }
        archive_entry_set_pathname(entry, rel_path);

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

void extract_archive(const char *path, const char *outpath) {
    struct archive *a_read;
    struct archive *a_write;
    struct archive_entry *entry;

    a_read = archive_read_new();
    archive_read_support_filter_zstd(a_read);
    archive_read_support_format_tar(a_read);

    a_write = archive_write_disk_new();
    archive_write_disk_set_options(a_write,

        ARCHIVE_EXTRACT_SECURE_NODOTDOT
        | ARCHIVE_EXTRACT_SECURE_SYMLINKS
        | ARCHIVE_EXTRACT_SECURE_NOABSOLUTEPATHS
        | ARCHIVE_EXTRACT_TIME
        | ARCHIVE_EXTRACT_PERM
    );

    archive_read_open_filename(a_read, path, 65536);
    while (archive_read_next_header(a_read, &entry) == ARCHIVE_OK) {
        char new_path[PATH_MAX];
        snprintf(new_path, sizeof(new_path), "%s/%s", outpath, archive_entry_pathname(entry));
        archive_entry_set_pathname(entry, new_path);

        archive_write_header(a_write, entry);
        if (archive_entry_size(entry) > 0) {
            const void *buffer;
            size_t bytes_read;
            int64_t offset;
            while (archive_read_data_block(a_read, &buffer, &bytes_read, &offset) == ARCHIVE_OK) {
                archive_write_data_block(a_write, buffer, bytes_read, offset);
            }
        }

        archive_write_finish_entry(a_write);
    }

    archive_read_close(a_read);
    archive_read_free(a_read);
    archive_write_close(a_write);
    archive_write_free(a_write);
}