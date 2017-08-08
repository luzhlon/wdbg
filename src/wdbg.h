#ifndef __WDBG_H__
#define __WDBG_H__

#include <winsock2.h>
#include <cassert>
#include <DbgEng.h>
#include <srpc.h>

#include <xval_val.h>

#ifdef _WDBG_EXPORT
#define WDBGEXPORT __declspec(dllexport)
#else
#define WDBGEXPORT __declspec(dllimport)
#endif // _WDBG_EXPORT


#define DbgClient IDebugClient5
#define DbgControl IDebugControl4
#define DbgSpaces IDebugDataSpaces4
#define DbgRegs IDebugRegisters2
#define DbgSyms IDebugSymbols3
#define DbgSysobj IDebugSystemObjects4

extern DbgClient WDBGEXPORT *g_client;
extern DbgControl WDBGEXPORT *g_ctrl;
extern DbgSpaces WDBGEXPORT *g_spaces;
extern DbgRegs WDBGEXPORT *g_regs;
extern DbgSyms WDBGEXPORT *g_syms;
extern DbgSysobj WDBGEXPORT *g_sysobj;
extern HRESULT g_hresult;
extern srpc::Session *g_ss;

struct FuncItem {
    const char *name;
    srpc::Function function;
};

namespace wdbg {
    using namespace xval;
    using namespace srpc;

    void init();
    void DValue2XValue(DEBUG_VALUE *p, Value *o, size_t n = 1);
    Value DValue2XValue(DEBUG_VALUE& p);

    int WDBGEXPORT load_functions(FuncItem* items);
}

extern "C" typedef FuncItem *(*WDBG_INIT_FUNC)(DbgClient*, DbgControl*, DbgSpaces*, DbgRegs*, DbgSyms*, DbgSysobj*);

#endif /* __WDBG_H__ */
