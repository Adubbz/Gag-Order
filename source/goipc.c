#include "goipc.h"

Result goipcInitialize(void)
{
    Result rc;

    if (R_FAILED(rc = smGetService(&g_pmshellSrv, "pm:shell")))
    {
        printf("Could not obtain pm:shell service. Error code: 0x%02x\n", rc);
        return rc;
    }

    if (R_FAILED(rc = smGetService(&g_setsysSrv, "set:sys")))
    {
        printf("Could not obtain set:sys service. Error code: 0x%02x\n", rc);
        return rc;
    }

    return rc;
}

Result nsvmInitialize(void)
{
    Result rc;

    if (R_FAILED(rc = smGetService(&g_nsvmSrv, "ns:vm")))
    {
        printf("Could not obtain ns:vm service. Error code: 0x%02x\n", rc);
        return rc;
    }

    return rc;
}

void goipcExit(void)
{
    serviceClose(&g_pmshellSrv);
    serviceClose(&g_setsysSrv);
}

void nsvmExit(void)
{
    serviceClose(&g_nsvmSrv);
}

/* 
======================
   ns:vm commands
======================
*/

Result nsvmNeedsUpdateVulnerability(u8 *out)
{
    IpcCommand c;
    ipcInitialize(&c);

    struct {
        u64 magic;
        u64 cmd_id;
    } *raw;

    raw = ipcPrepareHeader(&c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 1200;

    Result rc = serviceIpcDispatch(&g_nsvmSrv);

    if (R_SUCCEEDED(rc)) 
    {
        IpcParsedCommand r;
        ipcParse(&r);

        struct 
        {
            u64 magic;
            u64 result;
            u8 out;
        } *resp = r.Raw;

        rc = resp->result;
        *out = resp->out;
    }

    return rc;
}

Result nsvmGetSafeSystemVersion(u16 *out)
{
    IpcCommand c;
    ipcInitialize(&c);

    struct {
        u64 magic;
        u64 cmd_id;
    } *raw;

    raw = ipcPrepareHeader(&c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 1202;

    Result rc = serviceIpcDispatch(&g_nsvmSrv);

    if (R_SUCCEEDED(rc)) 
    {
        IpcParsedCommand r;
        ipcParse(&r);

        struct 
        {
            u64 magic;
            u64 result;
            u16 out;
        } *resp = r.Raw;

        rc = resp->result;
        *out = resp->out;
    }

    return rc;
}

/* 
======================
   pm:shell commands
======================
*/

Result pmshellTerminateProcessByTitleId(u64 tid)
{
    IpcCommand c;
    ipcInitialize(&c);

    struct {
        u64 magic;
        u64 cmd_id;
        u64 tid;
    } *raw;

    raw = ipcPrepareHeader(&c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 2;
    raw->tid = tid;

    Result rc = serviceIpcDispatch(&g_pmshellSrv);

    if (R_SUCCEEDED(rc)) 
    {
        IpcParsedCommand r;
        ipcParse(&r);

        struct 
        {
            u64 magic;
            u64 result;
        } *resp = r.Raw;

        rc = resp->result;
    }

    return rc;
}

/* 
======================
   set:sys commands
======================
*/

Result setsysGetSettingsItemValue(const char *name, const char *item_key, u64 *value_out)
{
    IpcCommand c;
    ipcInitialize(&c);
    ipcAddSendStatic(&c, name, SET_MAX_NAME_SIZE, 0);
    ipcAddSendStatic(&c, item_key, SET_MAX_NAME_SIZE, 0);
    ipcAddRecvBuffer(&c, value_out, sizeof(u64), 0);

    struct {
        u64 magic;
        u64 cmd_id;
    } *raw;

    raw = ipcPrepareHeader(&c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 38;

    Result rc = serviceIpcDispatch(&g_setsysSrv);

    if (R_SUCCEEDED(rc)) 
    {
        IpcParsedCommand r;
        ipcParse(&r);

        struct 
        {
            u64 magic;
            u64 result;
        } *resp = r.Raw;

        rc = resp->result;
    }

    return rc;
}

Result setsysGetSettingsItemValueSize(const char *name, const char *item_key, u64 *size_out)
{
    IpcCommand c;
    ipcInitialize(&c);
    ipcAddSendStatic(&c, name, SET_MAX_NAME_SIZE, 0);
    ipcAddSendStatic(&c, item_key, SET_MAX_NAME_SIZE, 0);

    struct {
        u64 magic;
        u64 cmd_id;
    } *raw;

    raw = ipcPrepareHeader(&c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 37;

    Result rc = serviceIpcDispatch(&g_setsysSrv);

    if (R_SUCCEEDED(rc)) 
    {
        IpcParsedCommand r;
        ipcParse(&r);

        struct 
        {
            u64 magic;
            u64 result;
            u64 size;
        } *resp = r.Raw;

        rc = resp->result;
        *size_out = resp->size;
    }

    return rc;
}