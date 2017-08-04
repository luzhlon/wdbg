
#include "xval_str.h"

#include "wdbg.h"
#include "handler.h"

DbgSpaces *g_spaces;

static void ReadVirtual(Session& rpc, Tuple& args) {
    ULONG64 offset = args[0].Int(0);
    if (offset) {
        ULONG size = args[1].Int(0);
        ULONG bytesread;
        auto buf = (char *)alloca(size);
        g_hresult = g_spaces->ReadVirtual(offset, buf, size, &bytesread);
        rpc.retn(S_OK == g_hresult ? String::New(buf, bytesread, true) : Value());
    }
}

static void WriteVirtual(Session& rpc, Tuple& args) {
    ULONG64 offset = args[0].Int(0);
    auto data = args[1];
    if (offset && data.isstr()) {
        ULONG byteswritten;
        g_hresult = g_spaces->WriteVirtual(
            offset,
            (char *)data.str().c_str(),
            data.str().size(),
            &byteswritten);
        if (S_OK == g_hresult)
            rpc.retn((uint64_t)byteswritten);
    }
}

static void ReadStringVirtual(Session& rpc, Tuple& args) {
    ULONG64 offset = args[0].Int(0);
    ULONG len;
    char buf[10240];
    g_hresult = g_spaces->ReadMultiByteStringVirtual(offset, sizeof(buf), buf, sizeof(buf), &len);
    rpc.retn(S_OK == g_hresult ? String::New(buf, len, true) : Value());
}

static void ReadUnicodeVirtual(Session& rpc, Tuple& args) {
    ULONG64 offset = args[0].Int(0);
    ULONG codePage = args[1].Int(CP_ACP);
    ULONG len = 0;
    char buf[10240];
    g_hresult = g_spaces->ReadUnicodeStringVirtual(offset, sizeof(buf), codePage, buf, sizeof(buf), &len);
    // return S_OK == g_hresult ? String::New(buf, len, true) : Value();
    rpc.retn(len ? String::New(buf, strlen(buf), true) : Value());
}

static void ReadPointersVirtual(Session& rpc, Tuple& args) {
    ULONG64 offset = args[0].Int(0);
    ULONG count = args[1].Int(0);
    auto p = (ULONG64 *)alloca(sizeof(ULONG64) * count);
    g_hresult = g_spaces->ReadPointersVirtual(count, offset, p);
    if (S_OK == g_hresult) {
        auto t = Tuple::New(count);
        for (size_t i = 0; i < count; i++)
            t.tuple().set(i, p[i]);
        rpc.retn(t);
    }
}

static void WritePointersVirtual(Session& rpc, Tuple& args) {
}

static void SearchVirtual(Session& rpc, Tuple& args) {
    ULONG64  Offset = 0;
    ULONG64  Length = 0;
    ULONG    Flags = 0;
    PVOID    Pattern = 0;
    ULONG    PatternSize = 0;
    ULONG    PatternGranularity = 1;

    ULONG64 MatchOffset;
    if (g_spaces->SearchVirtual2(Offset, Length, Flags, Pattern,
                PatternSize, PatternGranularity, &MatchOffset) == S_OK)
        rpc.retn(MatchOffset);
}

FuncItem debug_spaces_funcs[] = {
    {"ReadVirtual", ReadVirtual},
    {"WriteVirtual", WriteVirtual},
    {"ReadStringVirtual", ReadStringVirtual},
    {"ReadUnicodeVirtual", ReadUnicodeVirtual},
    {"ReadPointersVirtual", ReadPointersVirtual},
    // {"SearchVirtual", SearchVirtual},
    {nullptr, nullptr}
};
