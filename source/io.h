#pragma once

#include <stdio.h>
#include <dirent.h>
#include <switch.h>

typedef struct
{
    char *name;
    u8 is_dir;
} dir_ent_t;

u8 verifyNagBytes(const char *save_path, u32 bytes);
u8 patchNagBytes(const char *save_path);

void copyFile(const char *src_path, const char *dst_path);
int listDir(const char *path, dir_ent_t **dir_entries_out);