#include "io.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUF_SIZE 0x50000

u8 verifyNagBytes(const char *save_path, u32 bytes)
{
    FILE *f = fopen(save_path, "rb");

    if (f == NULL)
    {
        printf("Failed to open save file to check bytes!\n");
        return 1;
    }

    fseek(f, 0x8, 0);

    u32 nagBytes;
    fread(&nagBytes, sizeof(u32), 1, f);
    fclose(f);

    if (nagBytes == bytes)
    {
        printf("Valid nag bytes detected: 0x%02x\n", nagBytes);
        return 0;
    }
    else
    {
        printf("Invalid nag bytes detected: 0x%02x\nBailing out...\n", nagBytes);
        return 1;
    }
}

u8 patchNagBytes(const char *save_path)
{
    FILE *f = fopen(save_path, "wb");

    if (f == NULL)
    {
        printf("Failed to open save file to patch bytes!\n");
        return 1;
    }

    fseek(f, 0x8, 0);
    u32 unnagBytes = 0x100000C8;
    printf("Patching nag bytes to 0x%02x\n", unnagBytes);
    fwrite(&unnagBytes, sizeof(u32), 1, f);
    fclose(f);

    fsdevCommitDevice("ns_ssversion");

    printf("Verifying patching...\n");
    if (verifyNagBytes(save_path, unnagBytes) == 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void copyFile(const char *src_path, const char *dst_path)
{
    FILE *src = fopen(src_path, "rb");
    FILE *dst = fopen(dst_path, "wb");

    if (src == NULL || dst == NULL)
    {
        return;
    }

    size_t size;
    u8 *buf = malloc(sizeof(u8) * BUF_SIZE);
    u64 offset = 0;
    
    while ((size = fread(buf, 1, BUF_SIZE, src)) > 0)
    {
        fwrite(buf, 1, size, dst);
        offset += size;
    }

    free(buf);
    fclose(src);
    fclose(dst);

    // Check if the dest path starts with ns_ssversion:/
    if (strncmp(dst_path, "ns_ssversion:/", strlen("ns_ssversion:/")) == 0)
    {
        fsdevCommitDevice("ns_ssversion");
    }
}

int listDir(const char *path, dir_ent_t **dir_entries_out)
{
    DIR *dir = opendir(path);

    if (dir == NULL)
        return 0;

    struct dirent *entry;
    int entry_count = 0;

    while ((entry = readdir(dir)))
    {
        entry_count++;
    }

    // rewinddir appears to be broken
    closedir(dir);
    dir = opendir(path);

    if (dir == NULL)
        return 0;

    dir_ent_t *dir_entries = malloc(sizeof(dir_ent_t) * entry_count);
    u32 offset = 0;

    while ((entry = readdir(dir)))
    {
        dir_ent_t *dir_ent = dir_entries + offset;
        dir_ent->name = entry->d_name;
        dir_ent->is_dir = entry->d_type == DT_DIR;
        offset++;
    }

    closedir(dir);
    *dir_entries_out = dir_entries;

    return entry_count;
}

Result createDir(const char *path)
{
    mkdir(path, 777);
    return 0;
}