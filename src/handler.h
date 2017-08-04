#ifndef __HANDLER_H__
#define __HANDLER_H__

/***********************************************
 * The RPC Handler for wdbg
 *
 * @file     handler.h
 * @author   luzhlon
 *
 */

#include <srpc.h>
#include <xval_val.h>

using namespace srpc;
using namespace xval;

struct FuncItem {
    const char *name;
    Function function;
};

extern Session *g_ss;

extern FuncItem debug_control_funcs[];
extern FuncItem debug_client_funcs[];
extern FuncItem debug_spaces_funcs[];
extern FuncItem debug_regs_funcs[];
extern FuncItem debug_syms_funcs[];
extern FuncItem debug_sysobj_funcs[];

void load_functions();

#endif /* __HANDLER_H__ */
