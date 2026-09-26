#define _GNU_SOURCE

#include "../package/package.h"
#include "../package/util.h"

#include "../status.h"
#include "../path.h"

#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <limits.h>

int install_dir(const char *dir, const char *pkgname) {
    // FILES OPENED
    FILE *destinations = NULL;
    FILE *metadata = NULL;
    FILE *srcfile = NULL;
    FILE *destfile = NULL;

    int err_code = 0;

    char path[PATH_MAX];   

    snprintf(path, sizeof(path), "%s/destinations", dir);
    destinations = fopen(path, "rb");
    if (!destinations) {
        return ERR_FOPEN;
    }

    snprintf(path, sizeof(path), "%s/metadata", dir);
    metadata = fopen(path, "rb");
    if (!destinations) {
        return ERR_FOPEN;
    }

    snprintf(path, sizeof(path), "%s/%s", PATH_INSTALLED_PKGS, pkgname);
    if (mkdir_p(path, 0755) < 0) {
        return ERR_MKDIR;
    } else {
        snprintf(path, sizeof(path), "%s/%s/metadata", PATH_INSTALLED_PKGS, pkgname);
        FILE *var_metadata = fopen(path, "wb");
        if (!var_metadata) {
            return ERR_FOPEN;
        }
        chmod(path, 0644);

        snprintf(path, sizeof(path), "%s/%s/destinations", PATH_INSTALLED_PKGS, pkgname);
        FILE *var_destinations = fopen(path, "wb");
        if (!var_destinations) {
            return ERR_FOPEN;
        }
        chmod(path, 0644);

        copy_file(destinations, var_destinations, UINT64_MAX);
        copy_file(metadata, var_metadata, UINT64_MAX);
        fclose(var_destinations);
        fclose(var_metadata);
    }

    fseek(destinations, 0, SEEK_SET);
    char line[256] = "";
    while (fgets(line, sizeof(line), destinations)) {
        char *src = line;
        char *del = strstr(line, "->");
        if (del != NULL) {
            *del = '\0';
            char *dest = del + 2;
            dest[strcspn(dest, "\r\n")] = '\0';

            size_t src_len = strlen(src);
            while (src_len > 0 && src[src_len - 1] == '/') {
                src[src_len - 1] = '\0';
                src_len--;
            }

            char srcpath[PATH_MAX];
            snprintf(srcpath, sizeof(srcpath), "%s/files/%s", dir, src);

            struct stat st;
            if (stat(srcpath, &st) < 0) {
                err_code = ERR_STAT;
                goto out;
            }

            if (S_ISREG(st.st_mode)) {
                char destpath[PATH_MAX];
                size_t dest_len = strlen(dest);

                if (dest_len > 0 && dest[dest_len - 1] == '/') {
                    const char *filename = strrchr(src, '/');

                    if (filename) {
                        filename++;
                    } else {
                        filename = src;
                    }

                    snprintf(destpath, sizeof(destpath), "%s%s", dest, filename);
                } else {
                    snprintf(destpath, sizeof(destpath), "%s", dest);
                }

                char parent[PATH_MAX];
                snprintf(parent, sizeof(parent), "%s", destpath);

                char *slash = strrchr(parent, '/');

                if (slash) {
                    *slash = '\0';

                    if (mkdir_p(parent, 0755) < 0) {
                        err_code = ERR_MKDIR;
                        goto out;
                    }
                }

                srcfile = fopen(srcpath, "rb");
                destfile = fopen(destpath, "wb");
                if (!(srcfile && destfile)) {
                    goto out;
                }
                
                printf("Installing %s into %s\n", srcpath, destpath);
                copy_file(srcfile, destfile, UINT64_MAX);

                fclose(srcfile);
                srcfile = NULL;
                fclose(destfile);
                destfile = NULL;

                chmod(destpath, st.st_mode & 07777);

            } else if (S_ISDIR(st.st_mode)) {
                char destdir[PATH_MAX];
                size_t dest_len = strlen(dest);

                if (dest_len > 0 && dest[dest_len - 1] == '/') {
                    const char *dirname = strrchr(src, '/');

                    if (dirname) {
                        dirname++;
                    } else {
                        dirname = src;
                    }

                    snprintf(destdir, sizeof(destdir), "%s%s", dest, dirname);
                } else {
                    snprintf(destdir, sizeof(destdir), "%s", dest);
                }

                printf("Installing %s into %s\n", srcpath, destdir);
                err_code = copy_recursive(srcpath, destdir);
                if (err_code != 0) {
                    goto out;
                }
            }
        }
    }

out:
    //remove(dir);
    if (destinations)
        fclose(destinations);
    if (metadata)
        fclose(metadata);
    if (srcfile)
        fclose(srcfile);
    if (destfile)
        fclose(destfile);

    return err_code;
}

int install_spk(const char *package, bool local) {
    if (local) {
        printf("Installing %s from local file.\n", package);

        char name[512] = "";
        FILE *spk = fopen(package, "rb");
        if (spk) {
            struct metadata *metadata = get_spk_metadata(spk);
            fclose(spk);

            snprintf(name, sizeof(name), "%s-%s", metadata->name, metadata->version);
            free(metadata);
        }
        
        int res;
        char *dir = extract_spk(package, &res);
        if (res == SPK_OK) {
            printf("install res: %d", install_dir(dir, name));
        } else {
            printf("Extract err: %d\n", res);
        }

        free(dir);
    } else {
        printf("Searching repositories for %s.\n", package);
    }

    return 0;
}

int install_cmd(int argc, char **argv) {
    bool yes = false;
    bool from_file = false;

    struct option long_options[] = {
        {"yes",       no_argument, NULL, 'y'},
        {"from-file", no_argument, NULL, 'f'},
        {NULL, 0, NULL, 0}
    };

    optind = 1;
    int opt;

    while ((opt = getopt_long(argc, argv, "yf", long_options, NULL)) != -1) {
        switch (opt) {
            case 'y':
                yes = true;
                break;
            case 'f':
                from_file = true;
                break;
            default:
                return -1;
        }
    }

    while (optind < argc) {
        install_spk(argv[optind++], from_file);
    }

    return 0;
}