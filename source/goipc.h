#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <switch.h>

#define SET_MAX_NAME_SIZE 0x30

bool g_nsvmInitialized;

Service g_nsvmSrv;
Service g_pmshellSrv;
Service g_setfdSrv;
Service g_setsysSrv;

Result goipcInitialize(void);
Result nsvmInitialize(void);
void goipcExit(void);
void nsvmExit(void);

Result nsvmNeedsUpdateVulnerability(u8 *out);
Result nsvmGetSafeSystemVersion(u16 *out);

Result pmshellTerminateProcessByTitleId(u64 tid);

Result setsysGetSettingsItemValue(const char *name, const char *item_key, u64 *value_out);
Result setsysGetSettingsItemValueSize(const char *name, const char *item_key, u64 *size_out);