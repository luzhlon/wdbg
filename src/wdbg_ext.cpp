
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

static void readntheader(ULONG64 mod, PIMAGE_NT_HEADERS pnt) {
    ULONG64 base = mod;
    DWORD peoffset = 0;
    g_spaces->ReadVirtual(base + offsetof(IMAGE_DOS_HEADER, e_lfanew),
        &peoffset, sizeof(peoffset), nullptr);
    g_spaces->ReadVirtual(base + peoffset, pnt, sizeof(IMAGE_NT_HEADERS), nullptr);
}
// ([module], attribute...)
static void peinfo(Session& rpc, Tuple& args) {
    ULONG64 base = 0;
    size_t index = 0;
    
    if (args[0].isint())
        base = args[0].Int(), index++;
    else
        g_syms->GetModuleByIndex(0, &base);

    IMAGE_NT_HEADERS ImgNt;
    readntheader(base, &ImgNt);

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

FuncItem wdbg_ext_funcs[] = {
    {"peinfo", peinfo},
    {nullptr, nullptr},
};
