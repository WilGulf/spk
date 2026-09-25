#ifndef SPK_FORMAT_H
#define SPK_FORMAT_H

#include <stdint.h>

#define SPK_MAGIC 0x53504B01

struct spk_header {
    uint32_t magic;

    uint64_t metadata_offset;
    uint64_t metadata_size;

    uint64_t map_offset;
    uint64_t map_size;

    uint64_t data_offset;
    uint64_t data_size;
};

struct metadata {
    char name[256];
    char version[64];
};

#endif