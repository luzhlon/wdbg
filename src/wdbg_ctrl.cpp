
#include "wdbg.h"
#include "handler.h"

#include <xval_str.h>

DbgControl *g_ctrl;

static void AddOptions(Session& rpc, Tuple& args) {
    auto opts = args[0];
    if (opts.isint()) {
        g_hresult = g_ctrl->AddEngineOptions((uint64_t)opts);
        rpc.retn(S_OK == g_hresult);
    }
}

static void Execute(Session& rpc, Tuple& args) {
    PCSTR cmd = args[0];
    ULONG flags = args[1].Int(DEBUG_EXECUTE_ECHO);
    if (cmd) {
        g_hresult = g_ctrl->Execute(DEBUG_OUTCTL_THIS_CLIENT, cmd, DEBUG_EXECUTE_ECHO);
        if (g_hresult == S_OK) {
            ULONG status;
            g_hresult = g_ctrl->GetExecutionStatus(&status);
            rpc.retn((uint64_t)status);
        }
    }
}

static void GetExecutionStatus(Session& rpc, Tuple& args) {
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

void SetInterrupt(Session& rpc, Tuple& args) {
    ULONG flag = args[0].Int(DEBUG_INTERRUPT_ACTIVE);
    rpc.retn(S_OK == (g_hresult = g_ctrl->SetInterrupt(flag)));
}

static void Assemble(Session& rpc, Tuple& args) {
    ULONG64 offset = args[0];
    auto str = args[1];
    if (str.isstr()) {
        ULONG64 endoffset;
        g_hresult = g_ctrl->Assemble(offset, str, &endoffset);
        if (S_OK == g_hresult)
            rpc.retn(endoffset);
    }
}

static void Disassemble(Session& rpc, Tuple& args) {
    const size_t BUFSIZE = 1024;
    char buffer[BUFSIZE];
    ULONG64 offset = args[0].Int(0);
    ULONG64 count = args[1].Int(1);
    ULONG64 endoffset;
    ULONG dissize;
    ULONG flags = 0;
    if (offset && g_ctrl->Disassemble(offset, flags, buffer,
        sizeof(buffer), &dissize, &endoffset) == S_OK) {
        rpc.retn(String::TRef(buffer, dissize));
    }
}

static void WaitForEvent(Session& rpc, Tuple& args) {
    g_hresult = g_ctrl->WaitForEvent(0, args[0].Int(INFINITE));
    rpc.retn((int64_t)g_hresult);
}

static void SetAsmOpts(Session& rpc, Tuple& args) {
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

static void AddBreakpoint(Session& rpc, Tuple& args) {
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

FuncItem debug_control_funcs[] = {
    {"AddOptions", AddOptions},
    {"GetExecutionStatus", GetExecutionStatus},
    {"GetPromptText", GetPromptText},
    {"WaitForEvent", WaitForEvent},
    {"SetInterrupt", SetInterrupt},
    {"Assemble", Assemble},
    {"Disassemble", Disassemble},
    {"Execute", Execute},
    {"Evaluate", [](Session& rpc, Tuple& args) {
        auto expr = args[0];
        ULONG desiredType = args[1].Int(DEBUG_VALUE_INVALID);
        DEBUG_VALUE dv;
        if (expr.isstr()) {
            g_hresult = g_ctrl->Evaluate(expr, desiredType, &dv, nullptr);
            rpc.retn(S_OK == g_hresult ? wdbg::DValue2XValue(dv) : false);
        }
    }},
    {"AddBreakpoint", AddBreakpoint},
    {"SetAsmOpts", SetAsmOpts},
    {nullptr, nullptr}
};
