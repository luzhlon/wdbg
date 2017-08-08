
#include "wdbg.h"
#include "xval_str.h"

DbgSysobj *g_sysobj = nullptr;

using namespace xval;
using namespace srpc;

static void getpsid(Session& rpc, Tuple& args) {
    ULONG id;
    g_hresult = g_sysobj->GetCurrentProcessId(&id);
    if (S_OK == g_hresult)
        rpc.retn((uint64_t)id);
}

static void getpeb(Session& rpc, Tuple& args) {
    ULONG64 peb;
    g_hresult = g_sysobj->GetCurrentProcessPeb(&peb);
    if (S_OK == g_hresult)
        rpc.retn(peb);
}

static void getteb(Session& rpc, Tuple& args) {
    ULONG64 teb;
    g_hresult = g_sysobj->GetCurrentThreadTeb(&teb);
    if (S_OK == g_hresult)
        rpc.retn(teb);
}

static void exename(Session& rpc, Tuple& args) {
    char path[1024];
    ULONG size;
    g_hresult = g_sysobj->GetCurrentProcessExecutableName(path, sizeof(path), &size);
    if (S_OK == g_hresult)
        rpc.retn(String::TRef(path, size));
}

FuncItem debug_sysobj_funcs[] = {
    {"getpsid", getpsid},
    {"getpeb", getpeb},
    {"getteb", getteb},
    {"exename", exename},
    {nullptr, nullptr}
};
