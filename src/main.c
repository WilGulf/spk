#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <getopt.h>
#include <stdbool.h>

#include "package/package.h"
#include "commands/commands.h"

void print_help(void) {
    
}

void print_version(void) {
    
}

int main(int argc, char **argv) {
    bool yes = false;
    bool from_file = false;
    const char *package = NULL;

    if (!argv[1]) {
        print_version();
        return 0;
    }

    if (!strcmp(argv[1], "extract")) {
        if (!argv[2]) {
            print_help();
            return 0;
        }
        
        extract_spk(argv[2]);
        return 0;
    }

    if (!strcmp(argv[1], "create")) {
        if (!argv[2]) {
            print_help();
            return 0;
        }

        create_spk(argv[2]);
        return 0;
    }

    int opt = 0;

    struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"version", no_argument, NULL, 'v'},
        {"yes", no_argument, NULL, 'y'},
        {"from-file", no_argument, NULL, 'f'},
        {NULL, 0, NULL, 0}
    };

    while ((opt = getopt_long(argc, argv, "hv", long_options, NULL)) != -1) {
        switch (opt) {
            case 'h':
                print_help();
                return 0;
            case 'v':
                print_version();
                return 0;
            default:
                abort();
        }
    }

    return 0;
}