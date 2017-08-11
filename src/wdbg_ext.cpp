
#include "wdbg.h"
#include <windows.h>
#include <xval_str.h>
#include <xval_list.h>

using namespace xval;
using namespace srpc;

Value EntryPoint = "EntryPoint"_x;
Value Machine = "Machine"_x;
Value Subsystem = "Subsystem"_x;
Value ImageSize = "ImageSize"_x;

// ([module], attribute...)
static void peinfo(Session& rpc, Tuple& args) {
    ULONG64 base = 0;
    size_t index = 0;
    
    if (args[0].isint())
        base = args[0].Int(), index++;
    else
        g_syms->GetModuleByIndex(0, &base);

    IMAGE_NT_HEADERS64 ImgNt;
    g_spaces->ReadImageNtHeaders(base, &ImgNt);

    if (ImgNt.Signature == IMAGE_NT_SIGNATURE) {        // "PE\0\0"
        auto t_ = Tuple::New(args.size() - index);
        auto& t = t_.tuple();
        for (size_t i = 0; i < args.size(); i++, index++) {
            auto v = args[index];
            if (v == EntryPoint)
                t.set(i, (uint64_t)ImgNt.OptionalHeader.AddressOfEntryPoint);
            else if (v == ImageSize)
                t.set(i, (uint64_t)ImgNt.OptionalHeader.SizeOfImage);
            else if (v == Machine)
                t.set(i, (uint64_t)ImgNt.FileHeader.Machine);
            else
                t.set(i, Value());
        }
        rpc.retn(t);
    }
}

static void loadplug(Session& rpc, Tuple& args) {
    const char *path = args[0];
    if (path) {
        HMODULE mod = LoadLibrary(path);
        rpc.retn(mod != NULL);
    }
}

string wdbgpath() {
    char buf[512];
    auto len = GetModuleFileName(GetModuleHandle(NULL), buf, sizeof(buf));
    if (len > sizeof(buf)) {
        string path(len, '\0');
        GetModuleFileName(GetModuleHandle(NULL), (char *)path.c_str(), path.size());
    } else {
        return buf;
    }
}

string wdbgdir() {
    auto path = wdbgpath();
    auto pos = path.rfind('\\');
    if (pos == string::npos)
        return "";
    path.resize(pos);
    return path;
}
// Get the path of wdbg.exe
static void wdbgpath(Session& rpc, Tuple& args) {
    auto path = wdbgpath();
    rpc.retn(String::TRef(path.c_str(), path.size()));
}
// Get the directory of wdbg.exe
static void wdbgdir(Session& rpc, Tuple& args) {
    auto dir = wdbgdir();
    rpc.retn(String::TRef(dir.c_str(), dir.size()));
}
// Get or set the environment variable
static void env(Session& rpc, Tuple& args) {
    auto e = args[0];
    if (args[1].isstr())
        rpc.retn((bool)SetEnvironmentVariable(e, args[1]));
    else
        rpc.retn(String::TRef(getenv(e)));
}

FuncItem wdbg_ext_funcs[] = {
    {"peinfo", peinfo},
    {"wdbgpath", wdbgpath},
    {"wdbgdir", wdbgdir},
    {"env", env},
    {"loadplug", loadplug},
    {"echo", [](Session& rpc, Tuple& args) {
        rpc.retn(&args);
    }},
    {nullptr, nullptr},
};
