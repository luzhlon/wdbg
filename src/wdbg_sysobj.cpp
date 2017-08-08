
#include "wdbg.h"
#include "xval_str.h"

DbgSysobj *g_sysobj = nullptr;

using namespace xval;
using namespace srpc;

static void psid(Session& rpc, Tuple& args) {
    ULONG id;
    if (args[0].isint()) {
        id = args[0].Int();
        g_hresult = g_sysobj->SetCurrentProcessId(id);
        rpc.retn((uint64_t)S_OK == g_hresult);
    } else {
        g_hresult = g_sysobj->GetCurrentProcessId(&id);
        if (S_OK == g_hresult)
            rpc.retn((uint64_t)id);
    }
}

static void thid(Session& rpc, Tuple& args) {
    ULONG id;
    if (args[0].isint()) {
        id = args[0].Int();
        g_hresult = g_sysobj->SetCurrentThreadId(id);
        rpc.retn((uint64_t)S_OK == g_hresult);
    } else {
        g_hresult = g_sysobj->GetCurrentThreadId(&id);
        if (S_OK == g_hresult)
            rpc.retn((uint64_t)id);
    }
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
    {"psid", psid},
    {"thid", thid},
    {"getpeb", getpeb},
    {"getteb", getteb},
    {"exename", exename},
    {nullptr, nullptr}
};
