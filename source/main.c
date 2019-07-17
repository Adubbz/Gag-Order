#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <switch.h>

#include "diagnostics.h"
#include "io.h"
#include "patching.h"

NagStatus g_NagStatus;

void printNagStatus()
{
    switch (g_NagStatus)
    {
        case UNKNOWN:
            printf("Cannot verify nag status as ns:vm could not be initialized.\nThis likely means that ns was already killed. Proceed with caution!\n");
            break;

        case NAGGED_NOSETTING:
            printf("This console has been supernagged!\n");
            break;

        case NAGGED_SETTING:
            printf("This console has been supernagged!\n");
            printf("WARNING: The setting vulnerability!needs_update_vulnerability_policy is set. This tool does not unset this, and Supernag may still occur as a consequence even after patching ns_ssversion's savedata.\n");
            break;

        case UNNAGGED:
            printf("This console doesn't have supernag!\n");
            break;

        case FAILED:
            printf("Supernag status checking failed!\n");
            break;
    }
}

void printUsage()
{
    printf("Usage:\n\n");
    
    if (g_FeatureMode == PATCH || g_FeatureMode == ALL)
        printf("[A] Patch\n");
    
    printf("[B] Exit\n");

    if (g_FeatureMode == UNPATCH || g_FeatureMode == ALL)
        printf("[X] Unpatch\n");

    if (g_FeatureMode == PATCH || g_FeatureMode == UNPATCH || g_FeatureMode == ALL)
        printf("[Y] Debug Info\n");
}

void printHeader(bool enablePrintUsage)
{
    printf("Gag Order 0.1.0 by Adubbz\n");
    printf("-------------------------\n");
    printNagStatus();
    printf("\n");
    if (enablePrintUsage)
        printUsage();
}

void loopInput()
{
    bool awaitContinue = false;

    while (appletMainLoop())
    {
        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (awaitContinue)
        {
            if (kDown & KEY_A)
            {
                consoleClear();
                printHeader(true);
                awaitContinue = false;
            }
        }
        else
        {
            if (kDown & KEY_B) 
                break;
            else if (g_FeatureMode != EXIT)
            {
                if ((g_FeatureMode == PATCH || g_FeatureMode == ALL) && kDown & KEY_A)
                {
                    consoleClear();
                    printHeader(false);
                    if (R_FAILED(patch()))
                    {
                        printf("Patching failed!\n");
                        printf("Please ensure you are on a supported firmware version and try restarting the application and/or your console.\n");
                        printf("\nPress B to exit.\n");
                        continue;
                    }

                    g_NagStatus = UNNAGGED;
                    awaitContinue = true;
                    printf("\nPress A to continue...\n");
                }
                else if ((g_FeatureMode == UNPATCH || g_FeatureMode == ALL) && kDown & KEY_X)
                {
                    consoleClear();
                    printHeader(false);
                    if (R_FAILED(unpatch()))
                    {
                        printf("Unpatching failed!\n");
                        printf("Please ensure you are on a supported firmware version and try restarting the application and/or your console.\n");
                        printf("\nPress B to exit.\n");
                        continue;
                    }

                    g_NagStatus = NAGGED_NOSETTING;
                    awaitContinue = true;
                    printf("\nPress A to continue...\n");
                }
                else if (kDown & KEY_Y)
                {
                    consoleClear();
                    printHeader(false);
                    printDebugInfo();
                    awaitContinue = true;
                    printf("\nPress A to continue...\n");
                }
            }
        }

        if (kDown & KEY_B) 
            break;

        consoleUpdate(NULL);
    }
}

int main(int argc, char **argv)
{
    consoleInit(NULL);

    Result rc;

    if (R_FAILED(rc = pmshellInitialize()))
    {
        printf("Failed to initialize pm:shell. Error code: 0x%08x\n", rc);
        goto loop_input;
    }

    if (R_FAILED(rc = setsysInitialize()))
    {
        printf("Failed to initialize set:sys. Error code: 0x%08x\n", rc);
        goto loop_input;
    }

    if (R_SUCCEEDED(nsvmInitialize()))
    {
        g_nsvmInitialized = true;
    }

    // TODO: Check console version and print warnings
    g_NagStatus = checkNagStatus();
    printHeader(false);

    switch (g_NagStatus)
    {
        case FAILED:
            printf("A major error has occurred and Gag Order cannot proceed.\nPlease ensure you are on a supported firmware version and try restarting the application and/or your console.\n");
            printf("If the issue persists, please report it on Github at https://github.com/Adubbz/Gag-Order/issues\n");
            g_FeatureMode = EXIT;
            break;

        case UNKNOWN:
            g_FeatureMode = ALL;
            break;

        case NAGGED_NOSETTING:
        case NAGGED_SETTING:
            g_FeatureMode = PATCH;
            break;

        case UNNAGGED:
            g_FeatureMode = UNPATCH;
            break;

        default:
            g_FeatureMode = EXIT;
    }

    loop_input:
    printUsage();
    loopInput();

    if (g_nsvmInitialized)
        nsvmExit();

    setsysExit();
    pmshellExit();
    consoleExit(NULL);
    return rc;
}

