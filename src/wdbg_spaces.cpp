
#include <xval_str.h>
#include "wdbg.h"

DbgSpaces *g_spaces = nullptr;

using namespace xval;
using namespace srpc;
// Read the virtual memory
static void read(Session& rpc, Tuple& args) {
    ULONG64 offset = args[0].Int(0);
    if (offset) {
        ULONG size = args[1].Int(0);
        ULONG bytesread;
        auto buf = (char *)alloca(size);
        g_hresult = g_spaces->ReadVirtual(offset, buf, size, &bytesread);
        if (S_OK == g_hresult) {
            auto data = String::TRef(buf, bytesread);
            data.str().isbin(true);
            rpc.retn(data);
        }
    }
}
// Write the virtual memory
static void write(Session& rpc, Tuple& args) {
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
// Read the physical memory
static void readphy(Session& rpc, Tuple& args) {
    auto pos = args[0];
    if (pos.isint()) {
        ULONG bytesread;
        ULONG size = args[1].Int(1);
        auto buf = (char *)alloca(size);
        g_hresult = g_spaces->ReadPhysical(pos.Int(), buf, size, &bytesread);
        if (S_OK == g_hresult) {
            auto data = String::TRef(buf, bytesread);
            data.str().isbin(true);
            rpc.retn(data);
        }
    }
}
// Write the physical memory
static void writephy(Session& rpc, Tuple& args) {
    auto pos = args[0], data = args[1];
    if (pos.isint() && data.isstr()) {
        ULONG byteswritten;
        g_hresult = g_spaces->WritePhysical(
            pos.Int(), data, data.str().size(), &byteswritten);
        if (S_OK == g_hresult)
            rpc.retn((uint64_t)byteswritten);
    }
}

static void readstr(Session& rpc, Tuple& args) {
    ULONG64 offset = args[0].Int(0);
    ULONG len;
    char buf[10240];
    g_hresult = g_spaces->ReadMultiByteStringVirtual(offset, sizeof(buf), buf, sizeof(buf), &len);
    if (S_OK == g_hresult) {
        auto str = String::TRef(buf, len);
        str.str().isbin(true);
        rpc.retn(str);
    }
}

static void readustr(Session& rpc, Tuple& args) {
    ULONG64 offset = args[0].Int(0);
    ULONG codePage = args[1].Int(CP_ACP);
    ULONG len = 0;
    char buf[10240];
    g_hresult = g_spaces->ReadUnicodeStringVirtual(offset, sizeof(buf), codePage, buf, sizeof(buf), &len);
    // return S_OK == g_hresult ? String::New(buf, len, true) : Value();
    rpc.retn(len ? String::New(buf, strlen(buf), true) : Value());
}

static void readptr(Session& rpc, Tuple& args) {
    ULONG64 offset = args[0].Int(0);
    ULONG count = args[1].Int(1);
    auto p = (ULONG64 *)alloca(sizeof(ULONG64) * count);
    g_hresult = g_spaces->ReadPointersVirtual(count, offset, p);
    if (S_OK == g_hresult) {
        if (count > 1) {
            auto t = Tuple::New(count);
            for (size_t i = 0; i < count; i++)
                t.tuple().set(i, p[i]);
            rpc.retn(t);
        } else {
            rpc.retn((uint64_t)*p);
        }
    }
}

static void search(Session& rpc, Tuple& args) {
    ULONG64  Offset = args[0].Int(0);
    ULONG64  Length = args[1].Int(0);
    ULONG    Flags = 0;
    PVOID    Pattern =  nullptr;
    ULONG    PatternSize = 0;
    ULONG    PatternGranularity = 1;

    if (args[2].isstr()) {
        ULONG64 MatchOffset;
        Pattern = (PVOID)args[2].str().c_str();
        PatternSize = args[2].str().size();
        if (g_spaces->SearchVirtual2(Offset, Length, Flags, Pattern,
                    PatternSize, PatternGranularity, &MatchOffset) == S_OK)
            rpc.retn(MatchOffset);
    }
}

FuncItem debug_spaces_funcs[] = {
    {"read", read},
    {"write", write},
    {"readphy", readphy},
    {"writephy", writephy},
    {"readstr", readstr},
    {"readustr", readustr},
    {"readptr", readptr},
    {"search", search},
    {nullptr, nullptr}
};
