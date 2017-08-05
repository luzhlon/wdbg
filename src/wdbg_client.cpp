
#include "wdbg.h"
#include "handler.h"
#include "callback.h"

DbgClient *g_client = nullptr;

static void create(Session& rpc, Tuple& args) {
    auto path = args[0];
    ULONG flags = args[1].Int(DEBUG_CREATE_PROCESS_NO_DEBUG_HEAP | DEBUG_PROCESS);
    if (path.isstr()) {
        g_hresult = g_client->CreateProcess(0, (PSTR)path.str().c_str(), flags);
    }
    rpc.retn((uint64_t)g_hresult);
}

static void attach(Session& rpc, Tuple& args) {
    auto p = args[0];
    if (p.isint()) {
        ULONG aflags = args[1].Int(DEBUG_ATTACH_INVASIVE_RESUME_PROCESS);
        g_hresult = g_client->AttachProcess(0, p.Int(), aflags);
    } else if (p.isstr()) {
        ULONG cflags = args[1].Int(DEBUG_CREATE_PROCESS_NO_DEBUG_HEAP | DEBUG_PROCESS);
        ULONG aflags = args[2].Int(DEBUG_ATTACH_INVASIVE_RESUME_PROCESS);
        g_hresult = g_client->CreateProcessAndAttach(0, (char *)(const char *)p, cflags, 0, aflags);
    }
    rpc.retn((uint64_t)g_hresult);
}

static void detach(Session& rpc, Tuple& args) {
    g_hresult = g_client->DetachCurrentProcess();
    rpc.retn((uint64_t)g_hresult);
}

static void terminate(Session& rpc, Tuple& args) {
    g_hresult = g_client->TerminateCurrentProcess();
    rpc.retn((uint64_t)g_hresult);
}

static void abandon(Session& rpc, Tuple& args) {
    g_hresult = g_client->AbandonCurrentProcess();
    rpc.retn((uint64_t)g_hresult);
}

static void exitcode(Session& rpc, Tuple& args) {
    ULONG code;
    if (S_OK == g_client->GetExitCode(&code))
        rpc.retn((uint64_t)code);
}

static void setinput(Session& rpc, Tuple& args) {
    // 
}

static void setoutput(Session& rpc, Tuple& args) {
    OutputCallback::onoutput = args[0];
}

static void setevent(Session& rpc, Tuple& args) {
    auto t = args[0];
    auto f = args[1];
    rpc.retn(EventCallback::RegisterEvent((ULONG)t.Int(), f));
}

FuncItem debug_client_funcs[] = {
    {"create", create},
    {"setinput", setinput},
    {"setoutput", setoutput},
    {"setevent", setevent},
    {"attach", attach}, {"detach", detach},
    {"terminate", terminate},
    {"abandon", abandon}, {"exitcode", exitcode},
    {nullptr, nullptr}
};
