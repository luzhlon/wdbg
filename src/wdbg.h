#ifndef __WDBG_H__
#define __WDBG_H__

#include <winsock2.h>
#include <cassert>
#include <DbgEng.h>

#include "xval_val.h"

#define DbgClient IDebugClient5
#define DbgControl IDebugControl4
#define DbgSpaces IDebugDataSpaces4
#define DbgRegs IDebugRegisters2
#define DbgSyms IDebugSymbols3
#define DbgSysobj IDebugSystemObjects4

extern DbgClient *g_client;
extern DbgControl *g_ctrl;
extern DbgSpaces *g_spaces;
extern DbgRegs *g_regs;
extern DbgSyms *g_syms;
extern DbgSysobj *g_sysobj;
extern HRESULT g_hresult;

namespace wdbg {
    using namespace xval;

    void init();
    void DValue2XValue(DEBUG_VALUE *p, Value *o, size_t n = 1);
    Value DValue2XValue(DEBUG_VALUE& p);
}

#endif /* __WDBG_H__ */
