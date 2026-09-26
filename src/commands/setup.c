#include "commands.h"

#include "../package/util.h"
#include "../path.h"
#include "../status.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>

int download_to_file(const char *url, FILE *file) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

        res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            return ERR_URL;
        }
    }

    return 0;
}

int setup_spk(void) {
    printf("Setting up\n");

    if (mkdir_p("/etc/spk", 0755) < 0) {
        return ERR_MKDIR;
    }

    FILE *spkconf = fopen(PATH_SPK_CONF, "wb");
    if (spkconf) {
        printf("Downloading spk conf\n");
        chmod(PATH_SPK_CONF, 0644);
        if (download_to_file(URL_CONFIG, spkconf) != 0) {
            fclose(spkconf);
            return ERR_URL;
        }
    } else {
        return ERR_FOPEN;
    }

    FILE *repoconf = fopen(PATH_REPO_CONF, "wb");
    if (repoconf) {
        printf("Downloading repo conf\n");
        chmod(PATH_REPO_CONF, 0644);
        if (download_to_file(URL_REPOS, repoconf) != 0) {
            fclose(repoconf);
            return ERR_URL;
        }

        fclose(repoconf);
    } else {
        return ERR_FOPEN;
    }

    if (mkdir_p(PATH_INSTALLED_PKGS, 0755) < 0) {
        return ERR_MKDIR;
    }

    return 0;
}