
#include "wdbg.h"
#include "handler.h"
#include "callback.h"

DbgClient *g_client = nullptr;

static void CreateProcess(Session& rpc, Tuple& args) {
    auto path = args[0];
    ULONG flags = args[1].Int(DEBUG_CREATE_PROCESS_NO_DEBUG_HEAP | DEBUG_PROCESS);
    if (path.isstr()) {
        g_hresult = g_client->CreateProcess(0, (PSTR)path.str().c_str(), flags);
        rpc.retn(g_hresult == S_OK);
    }
}

static void AttachProcess(Session& rpc, Tuple& args) {
    ULONG pid = args[0].Int(0);
    ULONG flags = args[1].Int(DEBUG_ATTACH_INVASIVE_RESUME_PROCESS);
    rpc.retn(S_OK == g_client->AttachProcess(0, pid, flags));
}

static void DetachProcesses(Session& rpc, Tuple& args) {
    rpc.retn(S_OK == g_client->DetachProcesses());
}

static void DetachCurrentProcess(Session& rpc, Tuple& args) {
    rpc.retn(S_OK == g_client->DetachCurrentProcess());
}

static void TerminateProcesses(Session& rpc, Tuple& args) {
    rpc.retn(S_OK == g_client->TerminateProcesses());
}

static void GetExitCode(Session& rpc, Tuple& args) {
    ULONG code;
    if (S_OK == g_client->GetExitCode(&code))
        rpc.retn((uint64_t)code);
}

static void SetEventCallbacks(Session& rpc, Tuple& args) {
    ULONG mask = args[0].Int(
        DEBUG_EVENT_BREAKPOINT |
        DEBUG_EVENT_LOAD_MODULE |
        DEBUG_EVENT_EXCEPTION |
        DEBUG_EVENT_CREATE_THREAD |
        DEBUG_EVENT_EXIT_THREAD |
        DEBUG_EVENT_CREATE_PROCESS |
        DEBUG_EVENT_EXIT_PROCESS |
        DEBUG_EVENT_UNLOAD_MODULE |
        DEBUG_EVENT_SYSTEM_ERROR |
        // DEBUG_EVENT_SESSION_STATUS |
        // DEBUG_EVENT_CHANGE_DEBUGGEE_STATE |
        // DEBUG_EVENT_CHANGE_ENGINE_STATE |
        DEBUG_EVENT_CHANGE_SYMBOL_STATE
    );
    auto ecb = new EventCallback(mask);
    rpc.retn(S_OK == g_client->SetEventCallbacks(ecb));
}

static void RegisterOutput(Session& rpc, Tuple& args) {
    auto f = args[0];
    if (f.isstr()) {
        f.str().isbin(false);
        OutputCallback::onoutput = args[0];
        rpc.retn(true);
    }
}

static void RegisterEvent(Session& rpc, Tuple& args) {
    auto t = args[0];
    auto f = args[1];
    rpc.retn(EventCallback::RegisterEvent((ULONG)t.Int(), f));
}

FuncItem debug_client_funcs[] = {
    {"CreateProcess", CreateProcess},
    {"SetEventCallbacks", SetEventCallbacks},
    {"RegisterEvent", RegisterEvent},
    {"RegisterOutput", RegisterOutput},
    {"AttachProcess", AttachProcess},
    {"DetachProcesses", DetachProcesses},
    {"DetachCurrentProcess", DetachCurrentProcess},
    {"TerminateProcesses", TerminateProcesses},
    {"GetExitCode", GetExitCode},
    {nullptr, nullptr}
};
