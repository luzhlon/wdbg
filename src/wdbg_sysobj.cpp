
#include "wdbg.h"
#include "handler.h"
#include "xval_str.h"

DbgSysobj *g_sysobj = nullptr;

static void GetCurPsId(Session& rpc, Tuple& args) {
    ULONG id;
    g_hresult = g_sysobj->GetCurrentProcessId(&id);
    if (S_OK == g_hresult)
        rpc.retn((uint64_t)id);
}

static void GetCurPsPeb(Session& rpc, Tuple& args) {
    ULONG64 peb;
    g_hresult = g_sysobj->GetCurrentProcessPeb(&peb);
    if (S_OK == g_hresult)
        rpc.retn(peb);
}

static void GetCurPsExeName(Session& rpc, Tuple& args) {
    char path[1024];
    ULONG size;
    g_hresult = g_sysobj->GetCurrentProcessExecutableName(path, sizeof(path), &size);
    if (S_OK == g_hresult)
        rpc.retn(String::TRef(path, size));
}

FuncItem debug_sysobj_funcs[] = {
    {"GetCurPsId", GetCurPsId},
    {"GetCurPsPeb", GetCurPsPeb},
    {"GetCurPsExeName", GetCurPsExeName},
    {nullptr, nullptr}
};
