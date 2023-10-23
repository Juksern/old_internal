#include "hooks.hh"

gui* gGui = nullptr;
render* gDraw = nullptr;

font_handle_t Font;
bool init = false;

decltype(EndScene::m_original) EndScene::m_original;
auto EndScene::hooked(IDirect3DDevice9 *dev) -> HRESULT {
    HRESULT hr = m_original(dev);

    static void* gameoverlay = nullptr;
    if (!gameoverlay) {
        MEMORY_BASIC_INFORMATION info;
        VirtualQuery(__builtin_return_address(0), &info, sizeof(MEMORY_BASIC_INFORMATION));

        char mod[MAX_PATH];
        GetModuleFileNameA((HMODULE)info.AllocationBase, mod, MAX_PATH);

        if (strstr(mod, "gameoverlay"))
            gameoverlay = __builtin_return_address(0);
    }

    if (gameoverlay != __builtin_return_address(0))
        return hr;

    if (!init) {
        gDraw = new render(dev, 1500);
        Font = gDraw->create_font("Verdana", 8);
        gGui = new gui("ravage", 5.f, 5.f);
        gGui->initialize();
        init = true;
    } else {
        gDraw->begin();

        gGui->draw(gDraw);
        
        gDraw->draw();
        gDraw->end();
    }

    return hr;
}

decltype(Reset::m_original) Reset::m_original;
auto Reset::hooked(IDirect3DDevice9 *dev, D3DPRESENT_PARAMETERS *params) -> HRESULT {
    HRESULT hr = m_original(dev, params);

    if (gDraw) {
        gDraw->release();
        gDraw->reacquire();
    }

    if (hr == D3DERR_INVALIDCALL) {
        MessageBoxA(NULL, "invalid call", "Reset", MB_ICONERROR | MB_OK);
    }

    return hr;
}

decltype(DrawIndexedPrimitive::m_original) DrawIndexedPrimitive::m_original;
auto DrawIndexedPrimitive::hooked(IDirect3DDevice9 *dev, D3DPRIMITIVETYPE unnamedParam1, INT BaseVertexIndex, UINT MinVertexIndex,
                 UINT NumVertices, UINT startIndex, UINT primCount) -> HRESULT {
    return m_original(dev, unnamedParam1, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}