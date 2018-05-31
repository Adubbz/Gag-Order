#pragma once

#include <switch.h>

enum FeatureMode
{
    EXIT,
    PATCH,
    UNPATCH,
    ALL
} g_FeatureMode;

Result patch(void);
Result unpatch(void);

int verifyNagMagic(const char *save_path, u32 magic);
int patchNagBytes(const char *save_path, const u8 *bytes);