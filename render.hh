#pragma once
#include "includes.hh"

constexpr unsigned long vertex_definition = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;

namespace {
    inline bool is_type_list(D3DPRIMITIVETYPE type) {
        return type == D3DPT_POINTLIST ||
                type == D3DPT_LINELIST ||
                type == D3DPT_TRIANGLELIST;
    }

    inline int type_order(D3DPRIMITIVETYPE type) {
        switch (type) {
            case D3DPT_POINTLIST:
                return 1;
            case D3DPT_LINELIST:
            case D3DPT_LINESTRIP:
                return 2;
            case D3DPT_TRIANGLELIST:
            case D3DPT_TRIANGLESTRIP:
            case D3DPT_TRIANGLEFAN:
                return 3;
            default:
                return 0;
        }
    }
};

struct vertex_t {
    vertex_t() = default;

    vertex_t(float x, float y, float z, float r, D3DCOLOR color, float t_x, float t_y);
    vertex_t(float x, float y, float z, float r, D3DCOLOR color);
    vertex_t(float x, float y, float z, D3DCOLOR color);
    vertex_t(float x, float y, D3DCOLOR color);

    float m_x, m_y, m_z, m_r;
    D3DCOLOR m_color;
    float m_texture_x, m_texture_y;
};

struct batch_t {
    batch_t(size_t count, D3DPRIMITIVETYPE type, IDirect3DTexture9 *texture = nullptr);

    size_t m_count;
    D3DPRIMITIVETYPE m_type;
    IDirect3DTexture9 *m_texture;
};

struct font_handle_t {
    font_handle_t() = default;
    explicit font_handle_t(size_t id);

    size_t m_id;
};

class render;
class render_list;

enum font_flags_e : uint8_t {
    FONT_DEFAULT = 0 << 0,
    FONT_BOLD = 1 << 0,
    FONT_ITALIC = 1 << 1
};

enum text_flags_e : uint8_t {
	TEXT_LEFT         = 0 << 0,
	TEXT_RIGHT        = 1 << 1,
	TEXT_CENTERED_X   = 1 << 2,
	TEXT_CENTERED_Y   = 1 << 3,
	TEXT_CENTERED     = 1 << 2 | 1 << 3,
	TEXT_SHADOW       = 1 << 4,
	TEXT_COLORTAGS    = 1 << 5
};

class font {
public:
    font(render* renderer, IDirect3DDevice9 *device, const std::string &family, long height, uint8_t flags = FONT_DEFAULT);
    ~font();

    void draw_text(render_list* &render_queue, float x, float y, const std::string &text, D3DCOLOR color = D3DCOLOR_RGBA(255, 255, 255, 255), uint8_t flags = TEXT_LEFT);
    void get_text_extent(const std::string& text, float *w, float *h);

private:
    void create_gdi_font(HDC ctx, HGDIOBJ *gdi_font);
    HRESULT paint_alphabet(HDC ctx, bool measure_only = false);

	IDirect3DDevice9     *m_device;
	IDirect3DTexture9    *m_texture;
	long                  tex_width;
	long                  tex_height;
	float                 text_scale;
	float                 tex_coords[128 - 32][4];
	long                  m_spacing;

	std::string           m_family;
	long                  m_height;
	std::uint8_t          m_flags;

	render*               m_renderer;
};


class render_list {
public:
    render_list() = delete;
    render_list(size_t max_vertices);

    void clear();

    friend class render;

    std::vector<vertex_t> m_vertices;
    std::vector<batch_t> m_batches;
};

class render {
public:
    render(IDirect3DDevice9* device, size_t max_vertices);
    ~render();

    void reacquire();
    void release();

    void begin();
    void end();

    void draw();
    void draw(render_list* &render_queue);

    render_list* make_render_list();

    font_handle_t create_font(const std::string& family, long size, uint8_t flags = 0);

    template <size_t N>
    void add_vertices(render_list* &render_queue, vertex_t(&vertex_array)[N], D3DPRIMITIVETYPE type, IDirect3DTexture9* texture = nullptr);

    template <size_t N>
    void add_vertices(vertex_t(&vertex_array)[N], D3DPRIMITIVETYPE type, IDirect3DTexture9* texture = nullptr);

public:
    void rect(render_list* &render_queue, float x, float y, float w, float h, D3DCOLOR color);
    void rect(float x, float y, float w, float h, D3DCOLOR color);

    void box(render_list* &render_queue, float x, float y, float w, float h, D3DCOLOR color);
    void box(float x, float y, float w, float h, D3DCOLOR out_color);

    void get_text_extent(font_handle_t font, const std::string& text, float* w, float* h);

    void text(render_list* &render_queue, font_handle_t font, float x, float y, const std::string &text, D3DCOLOR color, uint8_t flags = 0);
    void text(font_handle_t font, float x, float y, const std::string &text, D3DCOLOR color, uint8_t flags = 0);

private:
    IDirect3DDevice9 *m_device;
    IDirect3DVertexBuffer9 *m_vertex_buffer;
    IDirect3DStateBlock9 *m_prev_state_block, *m_render_state_block;

    size_t m_max_vertices;
    render_list *m_render_queue;
    std::vector<font*> m_fonts;
};

template <size_t N>
inline void render::add_vertices(render_list *&render_queue, vertex_t (&vertex_array)[N], D3DPRIMITIVETYPE type, IDirect3DTexture9 *texture)
{
    size_t num_vertices = std::size(render_queue->m_vertices);
    if (std::empty(render_queue->m_batches) || 
        render_queue->m_batches.back().m_type != type ||
        render_queue->m_batches.back().m_texture != texture) {
        render_queue->m_batches.emplace_back(0, type, texture);
    }

    render_queue->m_batches.back().m_count += N;

    render_queue->m_vertices.resize(num_vertices + N);
    memcpy(&render_queue->m_vertices[std::size(render_queue->m_vertices) - N], &vertex_array[0], N * sizeof(vertex_t));

    switch (type) {
        case D3DPT_LINESTRIP:
        case D3DPT_TRIANGLESTRIP:
            render_queue->m_batches.emplace_back(0, D3DPT_FORCE_DWORD, nullptr);
        default:
            break;
    }
}

template <size_t N>
inline void render::add_vertices(vertex_t (&vertex_array)[N], D3DPRIMITIVETYPE type, IDirect3DTexture9 *texture)
{
    render::add_vertices(m_render_queue, vertex_array, type, texture);
}

extern render* gDraw;
