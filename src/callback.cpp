
#include "handler.h"
#include "callback.h"
#include "wdbg.h"
#include "xval_str.h"
#include "xval_list.h"

enum EV_INDEX {
    BREAKPOINT = 0,
    EXCEPTION,
    CREATEPROCESS,
    EXITPROCESS,
    CREATETHREAD,
    EXITTHREAD,
    LOADMODULE,
    UNLOADMODULE,
    SYSTEMERROR,

    EV_COUNT
};
// Event handler vector
Value ev = List::New(EV_COUNT);
// Output handler
Value OutputCallback::onoutput;

bool InputCallback::isinputting = false;

HRESULT InputCallback::StartInput(ULONG bufsize) {
    isinputting = true;
    auto str = g_ss->call("input", (uint64_t)bufsize);
    if (str.isstr()) {
        const char *p = str;
        const char *e = p + str.str().size();
        while (isinputting && p < e) {
            ULONG size;
            g_ctrl->Input((char *)p, e - p, &size);
            p += size;
        }
    } else
        printf("[Input] failure\n");
    return S_OK;
}

HRESULT InputCallback::EndInput() {
    isinputting = false; return S_OK;
}

HRESULT OutputCallback::Output(IN ULONG Mask, IN PCSTR Text) {
    if (onoutput.isnil()) return S_OK;
    auto str = String::TRef(Text);
    str.str().isbin(true);
    g_ss->notify(onoutput, str);
    return S_OK;
}

HRESULT EventCallback::GetInterestMask(PULONG Mask) {
    *Mask = _mask; return S_OK;
}

bool EventCallback::RegisterEvent(ULONG event, const Value& fid) {
    int p = -1;
    switch (event) {
    case DEBUG_EVENT_BREAKPOINT:
        p = BREAKPOINT; break;
    case DEBUG_EVENT_EXCEPTION:
        p = EXCEPTION; break;
    case DEBUG_EVENT_LOAD_MODULE:
        p = LOADMODULE; break;
    case DEBUG_EVENT_UNLOAD_MODULE:
        p = UNLOADMODULE; break;
    case DEBUG_EVENT_CREATE_THREAD:
        p = CREATETHREAD; break;
    case DEBUG_EVENT_EXIT_THREAD:
        p = EXITTHREAD; break;
    case DEBUG_EVENT_CREATE_PROCESS:
        p = CREATEPROCESS; break;
    case DEBUG_EVENT_EXIT_PROCESS:
        p = EXITPROCESS; break;
    }
    if (p < 0) return false;
    ev.list().set(p, fid);
    return true;
}

HRESULT EventCallback::Breakpoint(IDebugBreakpoint *bp) {
    // ??? return NOT_HANDLED will cause exception
    if (ev.list()[BREAKPOINT].isnil()) return DEBUG_STATUS_GO_NOT_HANDLED;
    ULONG64 offset; bp->GetOffset(&offset);
    auto ret = g_ss->call(ev.list()[BREAKPOINT], { (uint64_t)bp, offset });
    return ret.Int(DEBUG_STATUS_GO_NOT_HANDLED);
}

HRESULT EventCallback::ChangeDebuggeeState(ULONG Flags, ULONG64 Argument) {
    return DEBUG_STATUS_GO_NOT_HANDLED;
}

HRESULT EventCallback::ChangeEngineState(ULONG Flags, ULONG64 Argument) {
    return DEBUG_STATUS_GO_NOT_HANDLED;
}

HRESULT EventCallback::Exception(PEXCEPTION_RECORD64 Exception, ULONG FirstChance) {
    if (ev.list()[EXCEPTION].isnil()) return DEBUG_STATUS_GO_NOT_HANDLED;
    auto e = Dict::New();
    e.set("address", Exception->ExceptionAddress);
    e.set("code", (uint64_t)Exception->ExceptionCode);
    e.set("flags", (uint64_t)Exception->ExceptionFlags);
    e.set("information", Exception->ExceptionInformation);
    e.set("record", Exception->ExceptionRecord);
    e.set("first", (bool)FirstChance);
    return g_ss->call(ev.list()[EXCEPTION], e)
                        .Int(DEBUG_STATUS_GO_NOT_HANDLED);
}

HRESULT EventCallback::UnloadModule(PCSTR ImageBaseName, ULONG64 BaseOffset) {
    if (ev.list()[UNLOADMODULE].isnil()) return DEBUG_STATUS_GO_NOT_HANDLED;
    return g_ss->call(ev.list()[UNLOADMODULE],
        { String::TRef(ImageBaseName), (bool)BaseOffset })
                        .Int(DEBUG_STATUS_GO_NOT_HANDLED);
}

HRESULT EventCallback::ExitProcess(ULONG ExitCode) {
    if (ev.list()[EXITPROCESS].isnil()) return DEBUG_STATUS_GO_NOT_HANDLED;
    return g_ss->call(ev.list()[EXITPROCESS], (int64_t)ExitCode)
                        .Int(DEBUG_STATUS_GO_NOT_HANDLED);
}

HRESULT EventCallback::SessionStatus(ULONG Status) {
    return DEBUG_STATUS_GO_NOT_HANDLED;
}

HRESULT EventCallback::ChangeSymbolState(ULONG Flags, ULONG64 Argument) {
    return DEBUG_STATUS_GO_NOT_HANDLED;
}

HRESULT EventCallback::SystemError(ULONG Error, ULONG Level) {
    if (ev.list()[SYSTEMERROR].isnil()) return DEBUG_STATUS_GO_NOT_HANDLED;
    return g_ss->call(ev.list()[SYSTEMERROR],
                {(uint64_t)Error, (uint64_t)Level})
                .Int(DEBUG_STATUS_GO_NOT_HANDLED);
}

HRESULT EventCallback::CreateThread(
            ULONG64 Handle,
            ULONG64 DataOffset,
            ULONG64 StartOffset) {
    if (ev.list()[CREATETHREAD].isnil()) return DEBUG_STATUS_GO_NOT_HANDLED;
    return g_ss->call(ev.list()[CREATETHREAD],
        {Handle, DataOffset, StartOffset}).Int(DEBUG_STATUS_GO_NOT_HANDLED);
}

HRESULT EventCallback::ExitThread(ULONG ExitCode) {
    if (ev.list()[EXITTHREAD].isnil()) return DEBUG_STATUS_GO_NOT_HANDLED;
    return g_ss->call(ev.list()[EXITTHREAD], (int64_t)ExitCode)
                        .Int(DEBUG_STATUS_GO_NOT_HANDLED);
}

HRESULT EventCallback::LoadModule(
        IN ULONG64  ImageFileHandle,
        IN ULONG64  BaseOffset,
        IN ULONG  ModuleSize,
        IN PCSTR  ModuleName,
        IN PCSTR  ImageName,
        IN ULONG  CheckSum,
        IN ULONG  TimeDateStamp) {
    if (ev.list()[LOADMODULE].isnil()) return DEBUG_STATUS_GO_NOT_HANDLED;
    return g_ss->call(ev.list()[LOADMODULE], {
        ImageFileHandle,
        BaseOffset,
        (uint64_t)ModuleSize,
        String::TRef(ModuleName),
        String::TRef(ImageName),
        (uint64_t)CheckSum,
        (uint64_t)TimeDateStamp
    }).Int(DEBUG_STATUS_GO_NOT_HANDLED);
}

HRESULT EventCallback::CreateProcess(
        IN ULONG64  ImageFileHandle,
        IN ULONG64  Handle,
        IN ULONG64  BaseOffset,
        IN ULONG  ModuleSize,
        IN PCSTR  ModuleName,
        IN PCSTR  ImageName,
        IN ULONG  CheckSum,
        IN ULONG  TimeDateStamp,
        IN ULONG64  InitialThreadHandle,
        IN ULONG64  ThreadDataOffset,
        IN ULONG64  StartOffset) {
    if (ev.list()[CREATEPROCESS].isnil()) return DEBUG_STATUS_GO_NOT_HANDLED;
    return g_ss->call(ev.list()[CREATEPROCESS], {
        ImageFileHandle,
        Handle,
        BaseOffset,
        (uint64_t)ModuleSize,
        String::TRef(ModuleName),
        String::TRef(ImageName),
        (uint64_t)CheckSum,
        (uint64_t)TimeDateStamp,
        InitialThreadHandle,
        ThreadDataOffset,
        StartOffset
    }).Int(DEBUG_STATUS_GO_NOT_HANDLED);
}
