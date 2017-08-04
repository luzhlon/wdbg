
#include "wdbg.h"
#include "callback.h"

HRESULT g_hresult;
extern void load_functions();

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
        g_hresult = g_client->QueryInterface(__uuidof(DbgSyms), (void **)&g_sysobj);
        assert(g_hresult == S_OK);
        // Set callback object's
        g_client->SetOutputCallbacks(new OutputCallback());
        // Register the RPC functions
        load_functions();
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
}
