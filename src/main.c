#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <getopt.h>
#include <stdbool.h>
#include <unistd.h>

#include "package/package.h"
#include "commands/commands.h"
#include "package/util.h"

void print_help(void) {
    
}

void print_version(void) {
    
}

bool check_root(void) {
    if (getuid()) {
        return false;
    } else {
        return true;
    }
}

int main(int argc, char **argv) {
    bool yes = false;
    bool from_file = false;
    const char *package = NULL;

    if (!argv[1]) {
        print_version();
        return 0;
    }

    if (check_root()) {
        mkdir_p("/etc/spk", 0755);
    }

    if (!strcmp(argv[1], "extract")) {
        if (!check_root()) {
            printf("Please run extract with root privileges.\n");
            return 0;
        }

        if (!argv[2]) {
            print_help();
            return 0;
        }
        
        int res;
        free(extract_spk(argv[2], &res));

        if (res > 0) {
            printf("EXTRACT ERR: %d\n", res);
        }
        return 0;
    }

    if (!strcmp(argv[1], "create")) {
        if (!check_root()) {
            printf("Please run create with root privileges.\n");
            return 0;
        }

        if (!argv[2]) {
            print_help();
            return 0;
        }

        create_spk(argv[2]);
        return 0;
    }

    if (!strcmp(argv[1], "install")) {
        if (!check_root()) {
            printf("Please run install with root privileges.\n");
            return 0;
        }

        if (!argv[2]) {
            print_help();
            return 0;
        }

        install_cmd(argc - 1, argv + 1);
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