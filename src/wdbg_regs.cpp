
#include <string.h>
#include "wdbg.h"

DbgRegs *g_regs = nullptr;

using namespace wdbg;
using namespace xval;
using namespace srpc;

static bool GetRegIndexs(Value *p, ULONG *o, size_t n = 1) {
    for (size_t i = 0; i < n; i++) {
        if (p[i].isint())
            o[i] = (uint64_t)p[i];
        else if (p[i].isstr())
            g_regs->GetIndexByName(p[i], o + i);
        else
            return false;
    }
    return true;
}

static void XValue2DValue(Value &o, DEBUG_VALUE *p, ULONG type) {
    p->Type = type;
    switch (type) {
    case DEBUG_VALUE_INT8:
        p->I8 = (uint64_t)o; break;
    case DEBUG_VALUE_INT16:
        p->I16 = (uint64_t)o; break;
    case DEBUG_VALUE_INT32:
        p->I32 = (uint64_t)o; break;
    case DEBUG_VALUE_INT64:
        p->I64 = (uint64_t)o; break;
    case DEBUG_VALUE_FLOAT32:
        p->F32 = o; break;
    case DEBUG_VALUE_FLOAT64:
        p->F64 = o; break;
    }
}

// value, type
static void getreg(Session& rpc, Tuple& args) {
    ULONG index;
    DEBUG_VALUE val;
    if (GetRegIndexs(args.begin(), &index) && 
        (g_hresult = g_regs->GetValue(index, &val)) == S_OK)
        rpc.retn({ DValue2XValue(val), (uint64_t)val.Type});
}
// value, type
static void setreg(Session& rpc, Tuple& args) {
    ULONG index;
    DEBUG_VALUE dv;
    GetRegIndexs(args.begin(), &index);
    XValue2DValue(args[1], &dv, args[2].Int(DEBUG_VALUE_INT64));
    g_hresult = g_regs->SetValue(index, &dv);
    rpc.retn(S_OK == g_hresult);
}
// {value}
static void getregs(Session& rpc, Tuple& args) {
    auto indexs = (ULONG*)alloca(sizeof(ULONG) * args.size());
    auto dvs = (DEBUG_VALUE*)alloca(sizeof(DEBUG_VALUE) * args.size());
    GetRegIndexs(args.begin(), indexs, args.size());
    auto ret = Tuple::New(args.size());
    for (size_t i = 0; i < args.size(); i++)
        g_hresult = g_regs->GetValue(indexs[i], dvs+i);
    DValue2XValue(dvs, ret.tuple().begin(), args.size());
    rpc.retn(ret);
}

static void ipos(Session& rpc, Tuple& args) {
    ULONG source = args[0].Int(DEBUG_REGSRC_DEBUGGEE);
    ULONG64 offset;
    if (S_OK == g_regs->GetInstructionOffset2(source, &offset))
        rpc.retn(offset);
}

FuncItem debug_regs_funcs[] = {
    {"ipos", ipos},
    {"getreg", getreg},
    {"setreg", setreg},
    {"getregs", getregs},
    {nullptr, nullptr}
};
