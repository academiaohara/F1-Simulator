#include "f1_save.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    char magic[8];
    unsigned int version, size, checksum;
} SaveHeader;

static unsigned int checksum32(const void *data, unsigned int size) {
    const unsigned char *p = (const unsigned char *)data;
    unsigned int h = 2166136261u, i;
    for (i = 0; i < size; ++i) { h ^= p[i]; h *= 16777619u; }
    return h;
}

F1SaveResult f1_save_write(const char *path, const F1SaveState *state) {
    char tmp[512];
    SaveHeader h;
    FILE *f;
    if (!path || !state || strlen(path) + 5 >= sizeof tmp) return F1_SAVE_ERR_IO;
    snprintf(tmp, sizeof tmp, "%s.tmp", path);
    memcpy(h.magic, "F1PXSAV", 8);
    h.version = F1_SAVE_FILE_VERSION;
    h.size = (unsigned int)sizeof *state;
    h.checksum = checksum32(state, h.size);
    f = fopen(tmp, "wb");
    if (!f) return F1_SAVE_ERR_IO;
    if (fwrite(&h, sizeof h, 1, f) != 1 || fwrite(state, sizeof *state, 1, f) != 1 ||
        fflush(f) != 0 || fclose(f) != 0) {
        remove(tmp);
        return F1_SAVE_ERR_IO;
    }
    remove(path);
    if (rename(tmp, path) != 0) { remove(tmp); return F1_SAVE_ERR_IO; }
    return F1_SAVE_OK;
}

F1SaveResult f1_save_read(const char *path, F1SaveState *state) {
    SaveHeader h;
    F1SaveState temporary;
    FILE *f;
    int extra;
    if (!path || !state) return F1_SAVE_ERR_IO;
    f = fopen(path, "rb");
    if (!f) return F1_SAVE_ERR_IO;
    if (fread(&h, sizeof h, 1, f) != 1 || memcmp(h.magic, "F1PXSAV", 8) != 0 ||
        h.size != sizeof temporary) { fclose(f); return F1_SAVE_ERR_FORMAT; }
    if (h.version != F1_SAVE_FILE_VERSION) { fclose(f); return F1_SAVE_ERR_VERSION; }
    if (fread(&temporary, sizeof temporary, 1, f) != 1) { fclose(f); return F1_SAVE_ERR_FORMAT; }
    extra = fgetc(f);
    fclose(f);
    if (extra != EOF) return F1_SAVE_ERR_FORMAT;
    if (checksum32(&temporary, (unsigned int)sizeof temporary) != h.checksum)
        return F1_SAVE_ERR_CHECKSUM;
    *state = temporary;
    return F1_SAVE_OK;
}
