
#include <xval_str.h>
#include <xval_list.h>

#include "wdbg.h"
#include "callback.h"

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
Value ev = Tuple::New(EV_COUNT);
// Output handler
Value OutputCallback::onoutput;

bool InputCallback::isinputting = false;

HRESULT InputCallback::StartInput(ULONG bufsize) {
    isinputting = true;
    auto str = g_ss->call("Input", (uint64_t)bufsize);
    if (str.isstr()) {
        const char *p = str;
        const char *e = p + str.str().size();
        while (isinputting && p < e) {
            ULONG size;
            g_ctrl->ReturnInput(p);
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

namespace wdbg {
    HRESULT EventCallback::GetInterestMask(PULONG Mask) {
        *Mask = 
            DEBUG_EVENT_BREAKPOINT |
            DEBUG_EVENT_LOAD_MODULE |
            DEBUG_EVENT_EXCEPTION |
            DEBUG_EVENT_CREATE_THREAD |
            DEBUG_EVENT_EXIT_THREAD |
            DEBUG_EVENT_CREATE_PROCESS |
            DEBUG_EVENT_EXIT_PROCESS |
            DEBUG_EVENT_UNLOAD_MODULE |
            DEBUG_EVENT_SYSTEM_ERROR |
            DEBUG_EVENT_SESSION_STATUS |
            DEBUG_EVENT_CHANGE_DEBUGGEE_STATE |
            DEBUG_EVENT_CHANGE_ENGINE_STATE |
            DEBUG_EVENT_CHANGE_SYMBOL_STATE;
        return S_OK;
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
        ev.tuple().set(p, fid);
        return true;
    }

    HRESULT EventCallback::Breakpoint(IDebugBreakpoint *bp) {
        // ??? return NOT_HANDLED will cause exception
        auto h = ev.tuple()[BREAKPOINT];
        if (h.isnil())
            return DEBUG_STATUS_BREAK;
        ULONG64 offset; bp->GetOffset(&offset);
        return g_ss->call(h, {
            (uint64_t)bp, offset
        }).Int(DEBUG_STATUS_BREAK);
    }

    HRESULT EventCallback::ChangeDebuggeeState(ULONG Flags, ULONG64 Argument) {
        return DEBUG_STATUS_GO_NOT_HANDLED;
    }

    HRESULT EventCallback::ChangeEngineState(ULONG Flags, ULONG64 Argument) {
        return DEBUG_STATUS_GO_NOT_HANDLED;
    }

    HRESULT EventCallback::Exception(PEXCEPTION_RECORD64 Exception, ULONG FirstChance) {
        auto h = ev.tuple()[EXCEPTION];
        if (h.isnil())
            return DEBUG_STATUS_GO_NOT_HANDLED;
        return g_ss->call(h, {
            Exception->ExceptionAddress,
            (uint64_t)Exception->ExceptionCode,
            (uint64_t)Exception->ExceptionFlags,
            Exception->ExceptionInformation,
            Exception->ExceptionRecord,
            (bool)FirstChance
        }).Int(DEBUG_STATUS_GO_NOT_HANDLED);
    }

    HRESULT EventCallback::UnloadModule(PCSTR ImageBaseName, ULONG64 BaseOffset) {
        auto h = ev.tuple()[UNLOADMODULE];
        if (h.isnil()) return DEBUG_STATUS_GO_NOT_HANDLED;
        return g_ss->call(h, {
            String::TRef(ImageBaseName),
            (bool)BaseOffset
        }).Int(DEBUG_STATUS_GO_NOT_HANDLED);
    }

    HRESULT EventCallback::ExitProcess(ULONG ExitCode) {
        auto h = ev.tuple()[EXITPROCESS];
        if (h.isnil()) return DEBUG_STATUS_GO_NOT_HANDLED;
        return g_ss->call(h, (int64_t)ExitCode)
            .Int(DEBUG_STATUS_GO_NOT_HANDLED);
    }

    HRESULT EventCallback::SessionStatus(ULONG Status) {
        return DEBUG_STATUS_GO_NOT_HANDLED;
    }

    HRESULT EventCallback::ChangeSymbolState(ULONG Flags, ULONG64 Argument) {
        return DEBUG_STATUS_GO_NOT_HANDLED;
    }

    HRESULT EventCallback::SystemError(ULONG Error, ULONG Level) {
        auto h = ev.tuple()[SYSTEMERROR];
        if (h.isnil())
            return DEBUG_STATUS_BREAK;
        return g_ss->call(h, {
            (uint64_t)Error,
            (uint64_t)Level
        }).Int(DEBUG_STATUS_BREAK);
    }

    HRESULT EventCallback::CreateThread(
        ULONG64 Handle,
        ULONG64 DataOffset,
        ULONG64 StartOffset) {
        auto h = ev.tuple()[CREATETHREAD];
        if (h.isnil()) return DEBUG_STATUS_GO_NOT_HANDLED;
        return g_ss->call(h,
        { Handle, DataOffset, StartOffset }).Int(DEBUG_STATUS_GO_NOT_HANDLED);
    }

    HRESULT EventCallback::ExitThread(ULONG ExitCode) {
        auto h = ev.tuple()[EXITTHREAD];
        if (h.isnil()) return DEBUG_STATUS_GO_NOT_HANDLED;
        return g_ss->call(h, (int64_t)ExitCode)
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
        auto h = ev.tuple()[LOADMODULE];
        if (h.isnil()) return DEBUG_STATUS_GO_NOT_HANDLED;
        return g_ss->call(h, {
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
        auto h = ev.tuple()[CREATEPROCESS];
        if (h.isnil()) return DEBUG_STATUS_GO_NOT_HANDLED;
        return g_ss->call(h, {
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
}
