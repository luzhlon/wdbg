
#include <xval_str.h>
#include "wdbg.h"

DbgSyms *g_syms = nullptr;

using namespace xval;
using namespace srpc;

static void getoffsetbyname(Session& rpc, Tuple& args) {
    auto name = args[0];
    ULONG64 offset;
    if (name.isstr() && g_syms->GetOffsetByName(name, &offset) != S_FALSE)
        rpc.retn(offset);
}

static void symbolpath(Session& rpc, Tuple& args) {
    char buf[4096];
    ULONG size;
    if (args[0].isstr()) {
        g_hresult = g_syms->SetSymbolPath(args[0]);
        rpc.retn((uint64_t)g_hresult);
    } else {
        g_hresult == g_syms->GetSymbolPath(buf, sizeof(buf), &size);
        if (S_OK == g_hresult)
            rpc.retn(buf);
    }
}
// Get numbers of module
static void modnum(Session& rpc, Tuple& args) {
    ULONG loaded, unloaded;
    g_syms->GetNumberModules(&loaded, &unloaded);
    rpc.retn({ (uint64_t)loaded, (uint64_t)unloaded });
}
// Get module by id or name
static void getmod(Session& rpc, Tuple& args) {
    auto id = args[0];
    ULONG64 base = 0;
    if (id.isint()) {
        g_syms->GetModuleByIndex(id.Int(), &base);
        rpc.retn(base);
    } else if (id.isstr()) {
        ULONG index;
        g_syms->GetModuleByModuleName(id, args[1].Int(0), &index, &base);
        rpc.retn({ base, (uint64_t)index });
    }
}
// Get module by offset
static void ptr2mod(Session& rpc, Tuple& args) {
    ULONG64 base; ULONG index;
    g_syms->GetModuleByOffset(args[0].Int(), args[0].Int(0), &index, &base);
    rpc.retn({ base, (uint64_t)index });
}
// Get type's id by name and optional offset
static void typeid_(Session& rpc, Tuple& args) {
    ULONG64 mod = 0; ULONG id;
    const char *p = nullptr;
    if (args[0].isint())
        mod = args[0].Int(), p = args[1];
    else if (args[0].isstr())
        p = args[0];
    g_hresult = g_syms->GetTypeId(mod, p, &id);
    if (S_OK == g_hresult)
        rpc.retn((uint64_t)id);
}
//
static void symboltype(Session& rpc, Tuple& args) {
    ULONG id; ULONG64 mod;
    g_hresult = g_syms->GetSymbolTypeId(args[0], &id, &mod);
    if (S_OK == g_hresult)
        rpc.retn({ (uint64_t)id, mod });
}
// (id | name, module = 0)
static void typesize(Session& rpc, Tuple& args) {
    ULONG id = 0, size;
    auto n = args[0];
    if (n.isint()) id = n.Int();
    else if (n.isstr())
        g_syms->GetTypeId(args[1].Int(0), n, &id);
    g_syms->GetTypeSize(args[1].Int(0), id, &size);
    rpc.retn((uint64_t)size);
}
// get symbol type id
// get field offset
static void fieldoffset(Session& rpc, Tuple& args) {
    // ...
    ULONG64 module = 0;
    ULONG id = 0;
    if (args[0].isint()) id = args[0].Int();
    else if (args[0].isstr()) g_syms->GetTypeId(0, args[0], &id);
    ULONG offset;
    g_syms->GetFieldOffset(module, id, args[1], &offset);
    rpc.retn((uint64_t)offset);
}

static void fieldname(Session& rpc, Tuple& args) {
    char buf[1024];
    ULONG size;
    g_syms->GetFieldName(args[0].Int(), args[1].Int(), args[2].Int(), buf, sizeof(buf), &size);
    rpc.retn(String::TRef(buf, size));
}

FuncItem debug_syms_funcs[] = {
    {"getoffsetbyname", getoffsetbyname},
    {"symbolpath", symbolpath},
    {"modnum", modnum},
    {"getmod", getmod},
    {"ptr2mod", ptr2mod},
    {"typeid", typeid_},
    {"symboltype", symboltype},
    {"fieldoffset", fieldoffset},
    {nullptr, nullptr}
};
