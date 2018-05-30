#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <switch.h>
#include "goipc.h"
#include "io.h"

int main(int argc, char **argv)
{
    gfxInitDefault();
    consoleInit(NULL);
    
    Result rc;

    if (R_FAILED(rc = pmshellInitialize()))
    {
        printf("Failed to initialize pm:shell. Error code: 0x%02x\n", rc);
        goto await_input;
    }

    /*if (R_FAILED(rc = pmshellLaunchProcess(0, 0x010000000000001F, 3, 0)))
    {
        printf("Failed to launch ns. Error code: 0x%02x\n", rc);
        goto await_input;
    }*/

    if (R_FAILED(rc = goipcInitialize()))
    {
        printf("Failed to initialize goipc. Error code: 0x%02x\n", rc);
        goto await_input;
    }

    u8 nsvmInitialized = 0;

    if (R_SUCCEEDED(rc = nsvmInitialize()))
    {
        nsvmInitialized = 1;
    }

    u64 needs_update_setting;
    u64 needs_update_setting_size;
    u8 needs_update_cmd;
    u16 safe_system_version;

    if (R_SUCCEEDED(rc = setsysGetSettingsItemValueSize("vulnerability", "needs_update_vulnerability_policy", &needs_update_setting_size)))
        printf("vulnerability!needs_update_vulnerability_policy size: %lu\n", needs_update_setting_size);
    else
        printf("Failed to get setting size. Error code: 0x%02x\n", rc);

    if (R_SUCCEEDED(rc = setsysGetSettingsItemValue("vulnerability", "needs_update_vulnerability_policy", &needs_update_setting)))
        printf("vulnerability!needs_update_vulnerability_policy: %lu\n", needs_update_setting);
    else
        printf("Failed to get current value for needs_update_vulnerability_policy. Error code: 0x%02x\n", rc);

    if (nsvmInitialized)
    {
        if (R_SUCCEEDED(rc = nsvmNeedsUpdateVulnerability(&needs_update_cmd)))
            printf("nsvmNeedsUpdateVulnerability: %u\n", needs_update_cmd);
        else
            printf("Failed to get current value from nsvmNeedsUpdateVulnerability. Error code: 0x%02x\n", rc);

        if (R_SUCCEEDED(rc = nsvmGetSafeSystemVersion(&safe_system_version)))
            printf("nsvmGetSafeSystemVersion: %u\n", safe_system_version);
        else
            printf("Failed to get current value from nsvmGetSafeSystemVersion. Error code: 0x%02x\n", rc);
    }
    else
    {
        printf("Skipping ns:vm commands as it could not be initialized.\n");
    }

    if (R_SUCCEEDED(rc = pmshellTerminateProcessByTitleId(0x010000000000001F)))
        printf("Successfully terminated ns\n");
    else
    {
        printf("Failed to terminate ns. Error code: 0x%02x\n", rc);
        goto await_input;
    }

    FsFileSystem tmpMountedFs;

    if (R_SUCCEEDED(rc = fsMount_SystemSaveData(&tmpMountedFs, 0x8000000000000049)))
    {
        printf("Mounted ns_ssversion save data\n");
        printf("Handle: 0x%02x\n", tmpMountedFs.s.handle);
    }
    else
    {
        printf("Failed to mount system save data for ns_ssversion. Error code: 0x%02x\n", rc);
        goto await_input;
    }

    // TODO: Improve this check, seems to be wrong?
    if (fsdevMountDevice("ns_ssversion", tmpMountedFs) == -1) 
    {
        printf("Failed to mount system save data.\n");
        goto await_input;
    }
    else
    {
        printf("Successfully mounted ns_ssversion.\n");
    }

    printf("Backing up current ns_ssversion entry file to sd card...\n");
    copyFile("ns_ssversion:/entry", "sdmc:/entry-ns_ssversion_bak");
    printf("Done!\n");

    if (verifyNagBytes("ns_ssversion:/entry", 0x14020028) == 0)
    {
        patchNagBytes("ns_ssversion:/entry");
    }

    fsdevUnmountDevice("ns_ssversion");
    printf("Done!\n");

    await_input:
    printf("Press + to return to hbmenu.\n");
    while (appletMainLoop())
    {
        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (kDown & KEY_PLUS) break;

        gfxFlushBuffers();
        gfxSwapBuffers();
        gfxWaitForVsync();
    }

    goipcExit();

    if (nsvmInitialized)
        nsvmExit();

    pmshellExit();
    gfxExit();
    return rc;
}

