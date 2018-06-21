#include "patching.h"

#include <time.h>
#include "diagnostics.h"
#include "io.h"

const u8 NAGGED_BYTES[]   = { 0x16, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x28, 0x00, 0x02, 0x14, 0x03, 0x00, 0x00, 0x00 };
const u8 UNNAGGED_BYTES[] = { 0x16, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xC8, 0x00, 0x00, 0x10, 0x03, 0x00, 0x00, 0x00 };

Result patchInit(void)
{
    Result rc;
    FsFileSystem tmpMountedFs;

    printf("Attempting to terminate ns...\n");
    for (int attempt = 0; attempt < 100; attempt++)
    {
        pmshellTerminateProcessByTitleId(0x010000000000001F);

        if (R_SUCCEEDED(rc = fsMount_SystemSaveData(&tmpMountedFs, 0x8000000000000049)))
        {
            break;
        }
    }

    // Took too many attempts
    if (R_FAILED(rc))
    {
        printf("Failed to mount system save data %016lx. Error code: 0x%08x", 0x8000000000000049, rc);
        return rc;
    }

    if (g_nsvmInitialized)
    {
        g_nsvmInitialized = false;
    }

    // TODO: Improve this check, seems to be wrong?
    if (fsdevMountDevice("ns_ssversion", tmpMountedFs) == -1) 
    {
        printf("Failed to mount system save data.\n");
        rc = -1;
        return rc;
    }

    return rc;
}

Result patch(void)
{
    Result rc;

    if (R_FAILED(rc = patchInit()))
    {
        g_FeatureMode = EXIT;
        return rc;
    }

    printf("Backing up current ns_ssversion entry file to sd card...\n");
    copyFile("ns_ssversion:/entry", "sdmc:/entry-ns_ssversion-unpatched");

    if (verifyNagMagic("ns_ssversion:/entry", 0x14020028) == 0)
    {
        patchNagBytes("ns_ssversion:/entry", UNNAGGED_BYTES);
    }
    else
    {
        g_FeatureMode = EXIT;
    }

    copyFile("ns_ssversion:/entry", "sdmc:/entry-ns_ssversion-patched");
    fsdevUnmountDevice("ns_ssversion");
    printf("Done!\n");
    g_FeatureMode = UNPATCH;

    return rc;
}

Result unpatch(void)
{
    Result rc;

    if (R_FAILED(rc = patchInit()))
    {
        g_FeatureMode = EXIT;
        return rc;
    }

    printf("Backing up current ns_ssversion entry file to sd card...\n");
    copyFile("ns_ssversion:/entry", "sdmc:/entry-ns_ssversion-patched");

    if (verifyNagMagic("ns_ssversion:/entry", 0x100000C8) == 0)
    {
        patchNagBytes("ns_ssversion:/entry", NAGGED_BYTES);
    }
    else
    {
        g_FeatureMode = EXIT;
    }

    copyFile("ns_ssversion:/entry", "sdmc:/entry-ns_ssversion-unpatched");
    fsdevUnmountDevice("ns_ssversion");
    printf("Done!\n");
    g_FeatureMode = PATCH;

    return rc;
}

int verifyNagMagic(const char *save_path, u32 magic)
{
    FILE *f = fopen(save_path, "rb");

    if (f == NULL)
    {
        printf("Failed to open save file to check bytes!\n");
        return 1;
    }

    fseek(f, 0x8, 0);
    u32 nagMagic;
    fread(&nagMagic, sizeof(u32), 1, f);
    fclose(f);

    if (nagMagic == magic)
    {
        printf("Valid nag magic detected: 0x%08x\n", nagMagic);
        return 0;
    }
    else
    {
        printf("Invalid nag magic detected: 0x%08x. Expected 0x%08x\nBailing out...\n", nagMagic, magic);
        return 1;
    }
}

int patchNagBytes(const char *save_path, const u8 *bytes)
{
    FILE *f = fopen(save_path, "wb");

    if (f == NULL)
    {
        printf("Failed to open save file to patch bytes!\n");
        return 1;
    }

    printf("Patching nag bytes...\n");

    // The first version caused the end 0 padding to be trimmed
    // thus we need to recreate the file. It is not console unique,
    // and is mostly 0s.
    fwrite(bytes, 1, 0x10, f);
    u8 buff[0xFF0] = {0};
    fwrite(&buff, 1, 0xFF0, f);
    fclose(f);

    fsdevCommitDevice("ns_ssversion");

    printf("Verifying patching...\n");
    if (verifyNagMagic(save_path, *(u32 *)(bytes + 8)) == 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}