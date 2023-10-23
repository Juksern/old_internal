#pragma once
#include "includes.hh"

struct EndScene {
    static auto COM_DECLSPEC_NOTHROW STDMETHODCALLTYPE hooked (IDirect3DDevice9 *dev) -> HRESULT;
    static decltype(&hooked) m_original;
};

struct Reset {
    static auto COM_DECLSPEC_NOTHROW STDMETHODCALLTYPE hooked (IDirect3DDevice9 *dev, D3DPRESENT_PARAMETERS* params) -> HRESULT;
    static decltype(&hooked) m_original;
};

struct DrawIndexedPrimitive {
    static auto COM_DECLSPEC_NOTHROW STDMETHODCALLTYPE hooked (IDirect3DDevice9 *dev, D3DPRIMITIVETYPE unnamedParam1, INT BaseVertexIndex,UINT MinVertexIndex,
                                    UINT NumVertices,UINT startIndex,UINT primCount) -> HRESULT;
    static decltype(&hooked) m_original;
};