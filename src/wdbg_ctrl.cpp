
#include "wdbg.h"
#include "handler.h"

#include <xval_str.h>

DbgControl *g_ctrl;

static void addopts(Session& rpc, Tuple& args) {
    auto opts = args[0];
    if (opts.isint()) {
        g_hresult = g_ctrl->AddEngineOptions((uint64_t)opts);
        rpc.retn(S_OK == g_hresult);
    }
}

static void status(Session& rpc, Tuple& args) {
    ULONG status;
    g_hresult = g_ctrl->GetExecutionStatus(&status);
    rpc.retn((uint64_t)status);
}

static void GetPromptText(Session& rpc, Tuple& args) {
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
    rpc.retn((int64_t)g_hresult);
}

static void asmopts(Session& rpc, Tuple& args) {
    auto opt = args[0];
    if (opt.isint()) {
        g_hresult = g_ctrl->SetAssemblyOptions(opt.Int());
        rpc.retn(g_hresult == S_OK);
    }
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
        // ...
        rpc.retn((int64_t)bp);
    }
}

static void AssembleOptions(Session& rpc, Tuple& args) {
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

static HRESULT SetStatusAndRun(ULONG status) {
    g_hresult = g_ctrl->SetExecutionStatus(status);
    if (S_OK == g_hresult)
        g_hresult = g_ctrl->WaitForEvent(0, INFINITE);
    return g_hresult;
}

static void stepinto(Session& rpc, Tuple& args) {
    ULONG count = args[0].Int(1);
    while (count--)
        rpc.retn((uint64_t)SetStatusAndRun(DEBUG_STATUS_STEP_INTO));
}

static void stepover(Session& rpc, Tuple& args) {
    ULONG count = args[0].Int(1);
    while (count--)
        rpc.retn((uint64_t)SetStatusAndRun(DEBUG_STATUS_STEP_OVER));
}

static void run(Session& rpc, Tuple& args) {
    rpc.retn((uint64_t)SetStatusAndRun(DEBUG_STATUS_GO));
}

static void exec(Session& rpc, Tuple& args) {
    PCSTR cmd = args[0];
    ULONG flags = args[1].Int(DEBUG_EXECUTE_ECHO);
    if (cmd) {
        g_hresult = g_ctrl->Execute(DEBUG_OUTCTL_THIS_CLIENT, cmd, flags);
        if (g_hresult == S_OK) {
            ULONG status;
            g_hresult = g_ctrl->GetExecutionStatus(&status);
            if (status != DEBUG_STATUS_BREAK)
                g_ctrl->WaitForEvent(0, INFINITE);
        }
        rpc.retn((uint64_t)g_hresult);
    }
}

static void eval(Session& rpc, Tuple& args) {
    auto expr = args[0];
    ULONG desiredType = args[1].Int(DEBUG_VALUE_INVALID);
    DEBUG_VALUE dv;
    if (expr.isstr()) {
        g_hresult = g_ctrl->Evaluate(expr, desiredType, &dv, nullptr);
        rpc.retn(S_OK == g_hresult ? wdbg::DValue2XValue(dv) : false);
    }
}

FuncItem debug_control_funcs[] = {
    {"addopts", addopts},
    {"status", status},
    // {"GetPromptText", GetPromptText},
    // {"asm", asm},
    {"disasm", disasm},
    {"addbp", addbp},
    {"asmopts", asmopts},
    {"interrupt", interrupt},
    {"stepinto", stepinto},
    {"stepover", stepover},
    {"run", run},
    {"exec", exec},
    {"eval", eval},
    {"waitevent", waitevent},
    {nullptr, nullptr}
};
