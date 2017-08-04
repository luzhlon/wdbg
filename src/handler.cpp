
#include <xval_str.h>
#include <xval_list.h>
#include <xval_dict.h>

#include "wdbg.h"
#include "handler.h"

using namespace xval;

inline static void load_functions(FuncItem* items) {
    while (items->name)
        g_ss->addfunc(items->name, items->function),
        items++;
}

void load_functions() {
    load_functions(debug_control_funcs);
    load_functions(debug_client_funcs);
    load_functions(debug_spaces_funcs);
    load_functions(debug_regs_funcs);
    load_functions(debug_syms_funcs);
    load_functions(debug_sysobj_funcs);
}