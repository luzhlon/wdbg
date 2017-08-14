
#include <string>
#include "wdbg.h"
#include "callback.h"

HRESULT g_hresult;

extern FuncItem debug_control_funcs[];
extern FuncItem debug_client_funcs[];
extern FuncItem debug_spaces_funcs[];
extern FuncItem debug_regs_funcs[];
extern FuncItem debug_syms_funcs[];
extern FuncItem debug_sysobj_funcs[];
extern FuncItem wdbg_ext_funcs[];

void setextpath() {
    extern string wdbgpath();
    SetEnvironmentVariable("_NT_EXECUTABLE_IMAGE_PATH", wdbgpath().c_str());

    extern string wdbgdir();
    auto dir = wdbgdir();
    string t = dir + "\\winext;" + dir + "\\winxp;";
    SetEnvironmentVariable("_NT_DEBUGGER_EXTENSION_PATH", t.c_str());
}

namespace wdbg {
    using namespace xval;

    void init() {
        if (g_client) return;
        // Create debugger object's
        g_hresult = DebugCreate(__uuidof(DbgClient), (void **)&g_client);
        assert(g_hresult == S_OK);
        g_hresult = g_client->QueryInterface(__uuidof(DbgControl), (void **)&g_ctrl);
        assert(g_hresult == S_OK);
        g_hresult = g_client->QueryInterface(__uuidof(DbgRegs), (void **)&g_regs);
        assert(g_hresult == S_OK);
        g_hresult = g_client->QueryInterface(__uuidof(DbgSpaces), (void **)&g_spaces);
        assert(g_hresult == S_OK);
        g_hresult = g_client->QueryInterface(__uuidof(DbgSyms), (void **)&g_syms);
        assert(g_hresult == S_OK);
        g_hresult = g_client->QueryInterface(__uuidof(DbgSysobj), (void **)&g_sysobj);
        assert(g_hresult == S_OK);
        g_ctrl->AddEngineOptions(DEBUG_ENGOPT_INITIAL_BREAK | DEBUG_ENGOPT_FINAL_BREAK);
        g_ctrl->SetInterruptTimeout(1);
        // Set PATH
        setextpath();
        // Set callback object's
        g_client->SetInputCallbacks(new InputCallback);
        g_client->SetOutputCallbacks(new OutputCallback);
        g_client->SetEventCallbacks(new EventCallback);
        // Register the RPC functions
        load_functions(debug_control_funcs);
        load_functions(debug_client_funcs);
        load_functions(debug_spaces_funcs);
        load_functions(debug_regs_funcs);
        load_functions(debug_syms_funcs);
        load_functions(debug_sysobj_funcs);
        load_functions(wdbg_ext_funcs);
    }

    void DValue2XValue(DEBUG_VALUE *p, Value *o, size_t n) {
        for (size_t i = 0; i < n; i++)
            switch (p[i].Type) {
            case DEBUG_VALUE_INT8:
                o[i] = (uint64_t)p[i].I8; break;
            case DEBUG_VALUE_INT16:
                o[i] = (uint64_t)p[i].I16; break;
            case DEBUG_VALUE_INT32:
                o[i] = (uint64_t)p[i].I32; break;
            case DEBUG_VALUE_INT64:
                o[i] = (uint64_t)p[i].I64; break;
            case DEBUG_VALUE_FLOAT32:
                o[i] = p[i].F32; break;
            case DEBUG_VALUE_FLOAT64:
                o[i] = p[i].F64; break;
            default:
                o[i].reset(); break;
            }
    }

    Value DValue2XValue(DEBUG_VALUE& p) {
        Value v;
        DValue2XValue(&p, &v);
        return v;
    }

    int load_functions(FuncItem* items) {
        int i = 0;
        for (; items[i].name; i++)
            g_ss->addfunc(items[i].name, items[i].function);
        return i;
    }
}
