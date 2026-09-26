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

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0

#ifdef __i386__
    #define ARCH "i386"
#elif defined(__x86_64__)
    #define ARCH "x86_64"
#elif defined(__arm__)
    #define ARCH "arm"
#elif defined(__aarch64__)
    #define ARCH "aarch64"
#elif defined(__riscv)
    #define ARCH "RISC-V"
#elif defined(__powerpc__)
    #define ARCH "PowerPC"
#endif

void print_help(void) {
    printf("Usage: spk COMMAND [options]\n");
    putchar('\n');
    printf("List of Main Commands:\n");
    printf("    install <package> - Install package(s)\n");
    printf("    remove <package>  - Remove installed package(s)\n");
    printf("    update            - Update all repositories\n");
    printf("    upgrade           - Upgrade installed packages\n");
    printf("    search <query>    - Search for available package\n");
    printf("    info <package>    - Show package information\n");
    printf("    list              - List installed packages\n");
    printf("    inspect <package> - List files installed by package\n");
    printf("    setup             - Setup spk for this system\n");
    putchar('\n');
    printf("General spk options:\n");
    printf("    -v, --version   Print spk version\n");
    printf("    -h, --help      Print this help message\n");
    printf("    -y, --yes       Automatically answer yes for all questions\n");
    printf("    -f, --from-file Install package from local .spk file\n");
}

void print_version(void) {
    printf("spk version %d.%d.%d (%s)\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, ARCH);
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

    if (!strcmp(argv[1], "setup")) {
        if (!check_root()) {
            printf("Please run setup with root privileges.\n");
            return 0;
        }

        setup_spk();
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