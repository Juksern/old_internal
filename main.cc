#include "includes.hh"

VMT* d3d = nullptr;

void __stdcall cheat(void* module) {
    HMODULE mod = nullptr;

    while (!mod) {
        mod = GetModuleHandleA("shaderapidx9.dll");
    }

    MODULEINFO modinfo;
    if (!K32GetModuleInformation(GetCurrentProcess(), mod, &modinfo, sizeof(modinfo))) {
        MessageBoxA(NULL, "Failed to get module info", "cheat", MB_OK);
        return;
    }


    auto d3d9_addy = utils::find_pattern((PBYTE)modinfo.lpBaseOfDll, modinfo.SizeOfImage, "A1 ? ? ? ? 50 8B 08 FF 51 ? 85 C0");
    auto *device = **reinterpret_cast<IDirect3DDevice9***>(d3d9_addy + 1);

    if (!device) {
        MessageBoxA(NULL, "Failed to find device", "cheat", MB_OK);
        return;
    }

    d3d = new VMT(device);
    d3d->apply_hook<EndScene>(42);
    d3d->apply_hook<Reset>(16);
    d3d->apply_hook<DrawIndexedPrimitive>(82);

    while (GetAsyncKeyState(VK_END) & 0x01) {
        d3d->unhook();
        delete gGui;
        delete gDraw;
        delete d3d;
        FreeLibraryAndExitThread((HMODULE)module, 0);
    }
}

bool __stdcall DllMain(void* a, DWORD r, void* b) {
    if (r == DLL_PROCESS_ATTACH)
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)cheat, a, 0, nullptr);

    return true;
}