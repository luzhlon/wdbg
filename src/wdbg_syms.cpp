
#include "wdbg.h"
#include "handler.h"

DbgSyms *g_syms = nullptr;

static void GetOffsetByName(Session& rpc, Tuple& args) {
    auto name = args[0];
    ULONG64 offset;
    if (name.isstr() && g_syms->GetOffsetByName(name, &offset) != S_FALSE)
        rpc.retn(offset);
}

static void GetSymbolPath(Session& rpc, Tuple& args) {
    char buf[4096];
    ULONG size;
    if (S_OK == g_syms->GetSymbolPath(buf, sizeof(buf), &size))
        rpc.retn(buf);
}

FuncItem debug_syms_funcs[] = {
    {"GetOffsetByName", GetOffsetByName},
    {"GetSymbolPath", GetSymbolPath},
    {nullptr, nullptr}
};
