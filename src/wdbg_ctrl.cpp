
#include "wdbg.h"

#include <xval_str.h>

DbgControl *g_ctrl = nullptr;

using namespace xval;
using namespace srpc;

static void addopts(Session& rpc, Tuple& args) {
    auto opts = args[0];
    if (opts.isint()) {
        g_hresult = g_ctrl->AddEngineOptions((uint64_t)opts);
        rpc.retn(S_OK == g_hresult);
    }
}

static void status(Session& rpc, Tuple& args) {
    ULONG status;
    if (args[0].isint())
        g_hresult = g_ctrl->SetExecutionStatus(args[0].Int()),
        rpc.retn((uint64_t)g_hresult);
    else
        g_hresult = g_ctrl->GetExecutionStatus(&status),
        rpc.retn((uint64_t)status);
}

static void prompt(Session& rpc, Tuple& args) {
    char buf[1024];
    ULONG len;
    g_hresult = g_ctrl->GetPromptText(buf, sizeof(buf), &len);
    if (g_hresult == S_OK)
        rpc.retn(String::TRef(buf, len));
}

void interrupt(Session& rpc, Tuple& args) {
    ULONG flag = args[0].Int(DEBUG_INTERRUPT_ACTIVE);
    rpc.retn(S_OK == (g_hresult = g_ctrl->SetInterrupt(flag)));
}

static void asm_(Session& rpc, Tuple& args) {
    ULONG64 offset = args[0];
    auto str = args[1];
    if (str.isstr()) {
        ULONG64 endoffset;
        g_hresult = g_ctrl->Assemble(offset, str, &endoffset);
        if (S_OK == g_hresult)
            rpc.retn(endoffset);
    }
}

static void disasm(Session& rpc, Tuple& args) {
    char buffer[1024];
    ULONG64 offset = args[0].Int(0);
    ULONG64 count = args[1].Int(0);
    ULONG64 endoffset;
    ULONG dissize;
    ULONG flags = 0;
    if (!offset) return rpc.retn(false);
    if (count) {
        auto t = Tuple::New(count);
        endoffset = offset;
        for (size_t i = 0; i < count; i++) {
            g_hresult = g_ctrl->Disassemble(endoffset, flags,
                                buffer, sizeof(buffer), &dissize, &endoffset);
            if (S_OK == g_hresult)
                t.tuple().set(i, String::New(buffer, dissize));
            else
                break;
        }
        rpc.retn(t);
    } else {
        g_hresult = g_ctrl->Disassemble(offset, flags, buffer, sizeof(buffer), &dissize, &endoffset);
        if (S_OK == g_hresult)
            rpc.retn(String::TRef(buffer, dissize));
    }
}

static void waitevent(Session& rpc, Tuple& args) {
    g_hresult = g_ctrl->WaitForEvent(0, args[0].Int(INFINITE));
    rpc.retn((uint64_t)g_hresult);
}

auto type_s = "type"_x;
auto id_s = "id"_x;
auto enable_s = "enable"_x;
auto command_s = "command"_x;
auto thread_s = "thread"_x;
auto flags_s = "flags"_x;

static void addbp(Session& rpc, Tuple& args) {
    IDebugBreakpoint2 *bp;
    auto offset = args[0];
    auto opts = args[1];
    ULONG type = opts[type_s].Int(DEBUG_BREAKPOINT_CODE);
    ULONG id = opts[id_s].Int(DEBUG_ANY_ID);
    PCSTR cmd = opts[command_s];
    bool enable = opts[enable_s].Bool(true);
    ULONG thread = opts[thread_s].Int(0);
    g_hresult = g_ctrl->AddBreakpoint2(type, id, &bp);
    if (S_OK == g_hresult) {
        if (offset.isint())
            bp->SetOffset(offset);
        else if (offset.isstr())
            bp->SetOffsetExpression(offset);
        if (enable) bp->AddFlags(DEBUG_BREAKPOINT_ENABLED);
        if (cmd) bp->SetCommand(cmd);
        if (thread) bp->SetMatchThreadId(thread);
        if (type == DEBUG_BREAKPOINT_DATA) {
            bp->SetDataParameters(opts["access_size"_x].Int(1),
                opts["access_type"_x].Int(DEBUG_BREAK_READ));
        }
        if (S_OK == bp->GetId(&id))
            rpc.retn((uint64_t)id);
    }
}
// Remove a breakpoint
static void rmbp(Session& rpc, Tuple& args) {
    if (args[0].isint()) {
        ULONG id = args[0].Int();
        IDebugBreakpoint2 *bp;
        g_ctrl->GetBreakpointById2(id, &bp);
        g_hresult = g_ctrl->RemoveBreakpoint2(bp);
        rpc.retn((uint64_t)g_hresult);
    }
}
// Enable or disable a breakpoint
static void enablebp(Session& rpc, Tuple& args) {
    if (args[0].isint()) {
        ULONG id = args[0].Int();
        IDebugBreakpoint2 *bp;
        if (S_OK == g_ctrl->GetBreakpointById2(id, &bp)) {
            if (args[1].Bool(true))
                g_hresult = bp->AddFlags(DEBUG_BREAKPOINT_ENABLED);
            else
                g_hresult = bp->RemoveFlags(DEBUG_BREAKPOINT_ENABLED);
            rpc.retn((uint64_t)g_hresult);
        }
    }
}
// Get the position of instruction 'ret' in this function
static void retpos(Session& rpc, Tuple& args) {
    ULONG64 offset;
    g_hresult = g_ctrl->GetReturnOffset(&offset);
    if (S_OK == g_hresult)
        rpc.retn(offset);
}
// Get or set the assemble(disassemble) options
static void asmopts(Session& rpc, Tuple& args) {
    auto opt = args[0];
    if (opt.isint()) {
        g_hresult = g_ctrl->SetAssemblyOptions((uint64_t)opt);
        rpc.retn(g_hresult == S_OK);
    } else {
        ULONG options;
        g_hresult = g_ctrl->GetAssemblyOptions(&options);
        rpc.retn(g_hresult == S_OK ? (uint64_t)options : false);
    }
}
// Execute a command(windbg)
static void exec(Session& rpc, Tuple& args) {
    PCSTR cmd = args[0];
    ULONG flags = args[1].Int(DEBUG_EXECUTE_ECHO);
    if (cmd) {
        g_hresult = g_ctrl->Execute(DEBUG_OUTCTL_THIS_CLIENT, cmd, flags);
        rpc.retn((uint64_t)g_hresult);
    }
}
// Evaluate a expression
static void eval(Session& rpc, Tuple& args) {
    auto expr = args[0];
    ULONG desiredType = args[1].Int(DEBUG_VALUE_INVALID);
    DEBUG_VALUE dv;
    if (expr.isstr()) {
        g_hresult = g_ctrl->Evaluate(expr, desiredType, &dv, nullptr);
        if (S_OK == g_hresult)
            rpc.retn(wdbg::DValue2XValue(dv));
    }
}

static void is64(Session& rpc, Tuple& args) {
    rpc.retn(g_ctrl->IsPointer64Bit() == S_OK);
}

static void putstate(Session& rpc, Tuple& args) {
    g_hresult = g_ctrl->OutputCurrentState(DEBUG_OUTCTL_THIS_CLIENT, DEBUG_CURRENT_DEFAULT);
    rpc.retn((uint64_t)g_hresult);
}

static void addext(Session& rpc, Tuple& args) {
    ULONG64 handle;
    g_hresult = g_ctrl->AddExtension(args[0], 0, &handle);
    if (S_OK == g_hresult)
        rpc.retn(handle);
}

static void callext(Session& rpc, Tuple& args) {
    ULONG64 handle = args[0].Int(0);
    if (handle) {
        g_hresult = g_ctrl->CallExtension(handle, args[1], args[2]);
        rpc.retn((uint64_t)g_hresult);
    }
}

FuncItem debug_control_funcs[] = {
    {"addopts", addopts},
    {"status", status},
    {"prompt", prompt},
    {"putstate", putstate},
    // {"asm", asm},
    {"disasm", disasm},
    {"addbp", addbp},
    {"rmbp", addbp},
    {"enablebp", enablebp},
    {"retpos", retpos},
    {"asmopts", asmopts},
    {"interrupt", interrupt},
    {"exec", exec},
    {"eval", eval},
    {"waitevent", waitevent},
    {"is64", is64},
    {"addext", addext},
    {"callext", callext},
    {nullptr, nullptr}
};
