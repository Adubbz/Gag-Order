#include "diagnostics.h"

#include <stdio.h>

void printDebugInfo()
{
    Result rc;
    u16 safeSystemVersion;

    if (g_nsvmInitialized)
    {
        if (R_SUCCEEDED(rc = nsvmGetSafeSystemVersion(&safeSystemVersion)))
            printf("nsvmGetSafeSystemVersion: %u\n", safeSystemVersion);
        else
            printf("Failed to get current value from nsvmGetSafeSystemVersion. Error code: 0x%08x\n", rc);
    }
    else
        printf("Cannot get value of nsvmGetSafeSystemVersion as ns:vm hasn't been initialized.");
}

NagStatus checkNagStatus(void)
{
    if (!g_nsvmInitialized)
    {
        // ns:vm is not essential to proceed, however users need to be warned that this means there is less verification
        // in place.
        return UNKNOWN;
    }

    Result rc;
    bool needsUpdate;

    if (R_FAILED(rc = nsvmNeedsUpdateVulnerability(&needsUpdate)))
    {
        printf("Failed to get current value from nsvmNeedsUpdateVulnerability. Error code: 0x%08x\n", rc);
        return FAILED;
    }
    
    if (!needsUpdate)
        return UNNAGGED;

    u64 needsUpdateSettingSize;
    u64 needsUpdateSetting;

    if (R_FAILED(rc = setsysGetSettingsItemValueSize("vulnerability", "needs_update_vulnerability_policy", &needsUpdateSettingSize)))
    {
        printf("Could not get vulnerability!needs_update_vulnerability_policy size! Error code: 0x%08x\n", rc);
        return FAILED;
    }

    if (needsUpdateSettingSize != 4)
    {
        printf("Invalid needs_update_vulnerability_policy size %lu. This likely means you are on an unsupported firmware.", needsUpdateSettingSize);
        return FAILED;
    }

    if (R_FAILED(rc = setsysGetSettingsItemValue("vulnerability", "needs_update_vulnerability_policy", &needsUpdateSetting, sizeof(u64))))
    {
        printf("Could not get the value of vulnerability!needs_update_vulnerability_policy! Error code: 0x%08x\n", rc);
        return FAILED;
    }

    if (needsUpdateSetting)
        return NAGGED_SETTING;

    return NAGGED_NOSETTING;
}