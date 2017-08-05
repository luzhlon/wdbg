#ifndef __CALLBACK_H__
#define __CALLBACK_H__

#include <dbgeng.h>
#include <xval_val.h>
#include <xval_str.h>

#include "srpc.h"

using namespace xval;

class InputCallback : public IDebugInputCallbacks {
public:
    virtual ULONG _stdcall AddRef() { return 0; }
    virtual ULONG _stdcall Release() { return 1; }
    virtual HRESULT _stdcall QueryInterface(REFIID id, void **pp) {
        *pp = NULL;
        if (IsEqualIID(id, __uuidof(IUnknown)) ||
            IsEqualIID(id, __uuidof(IDebugInputCallbacks))) {
            *pp = this, AddRef();
            return S_OK;
        }
        else
            return E_NOINTERFACE;
    }

    static bool isinputting;

    virtual HRESULT _stdcall StartInput(ULONG bufsize);
    virtual HRESULT _stdcall EndInput();
};

class OutputCallback : public IDebugOutputCallbacks {
public:
    virtual ULONG _stdcall AddRef() { return 0; }
    virtual ULONG _stdcall Release() { return 1; }
    virtual HRESULT _stdcall QueryInterface(REFIID id, void **pp) {
        *pp = NULL;
        if (IsEqualIID(id, __uuidof(IUnknown)) ||
            IsEqualIID(id, __uuidof(IDebugOutputCallbacks))) {
            *pp = this, AddRef();
            return S_OK;
        } else
            return E_NOINTERFACE;
    }
    // Function's id to notify on output
    static Value onoutput;

    virtual HRESULT _stdcall Output(IN ULONG Mask, IN PCSTR Text);
};

class EventCallback : public IDebugEventCallbacks {
public:
    EventCallback(ULONG mask) { _mask = mask; }

    virtual ULONG _stdcall AddRef() { return 0; }
    virtual ULONG _stdcall Release() { return 1; }
    virtual HRESULT _stdcall QueryInterface(REFIID id, void **ppvObj) {
        *ppvObj = this; return NOERROR;
    }

    static bool RegisterEvent(ULONG event, const Value& fid);

    virtual HRESULT _stdcall GetInterestMask(PULONG Mask);

    virtual HRESULT _stdcall Breakpoint(IDebugBreakpoint *bp);
    virtual HRESULT _stdcall ChangeDebuggeeState(ULONG Flags, ULONG64 Argument);
    virtual HRESULT _stdcall ChangeEngineState(ULONG Flags, ULONG64 Argument);
    virtual HRESULT _stdcall Exception(PEXCEPTION_RECORD64 Exception, ULONG FirstChance);
    virtual HRESULT _stdcall UnloadModule(PCSTR ImageBaseName, ULONG64 BaseOffset);
    virtual HRESULT _stdcall ExitProcess(ULONG ExitCode);
    virtual HRESULT _stdcall SessionStatus(ULONG Status);
    virtual HRESULT _stdcall ChangeSymbolState(ULONG Flags, ULONG64 Argument);
    virtual HRESULT _stdcall SystemError(ULONG Error, ULONG Level);
    virtual HRESULT _stdcall CreateThread(
            ULONG64 Handle,
            ULONG64 DataOffset,
            ULONG64 StartOffset);
    virtual HRESULT _stdcall ExitThread(ULONG ExitCode);
    virtual HRESULT _stdcall LoadModule(
        IN ULONG64  ImageFileHandle,
        IN ULONG64  BaseOffset,
        IN ULONG  ModuleSize,
        IN PCSTR  ModuleName,
        IN PCSTR  ImageName,
        IN ULONG  CheckSum,
        IN ULONG  TimeDateStamp);

    virtual HRESULT _stdcall CreateProcess(
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
        IN ULONG64  StartOffset);

    ULONG _mask;
};

#endif /* __CALLBACK_H__ */
