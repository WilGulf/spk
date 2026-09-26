#ifndef TAR_H
#define TAR_H

void write_archive(const char *outname, const char *dirpath);
int extract_archive(const char *path, const char *outpath);

#endif