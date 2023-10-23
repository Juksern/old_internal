#include "render.hh"

render::render(IDirect3DDevice9 *device, size_t max_vertices)
    : m_device(device), m_vertex_buffer(nullptr), m_max_vertices(max_vertices),
    m_render_queue(new render_list(max_vertices)), m_prev_state_block(nullptr), m_render_state_block(nullptr)
{
    if (!m_device) {
        MessageBoxA(NULL, "device was nullptr", "renderer", MB_OK);
        return;
    }

    reacquire();
}

render::~render()
{
    delete m_render_queue;
    release();
}

void render::reacquire()
{
    if (FAILED(m_device->CreateVertexBuffer(m_max_vertices * sizeof(vertex_t), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
    vertex_definition, D3DPOOL_DEFAULT, &m_vertex_buffer, nullptr))) {
        MessageBoxA(NULL, "failed to create vertex buffer", "renderer::reacquire", MB_OK);
        return;
    }

    for (int i = 0; i < 2; ++i) {
        m_device->BeginStateBlock();

        m_device->SetRenderState(D3DRS_ZENABLE, FALSE);

		m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		m_device->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
		m_device->SetRenderState(D3DRS_ALPHAREF, 0x08);
		m_device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);

		m_device->SetRenderState(D3DRS_LIGHTING, FALSE);

		m_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		m_device->SetRenderState(D3DRS_STENCILENABLE, FALSE);
		m_device->SetRenderState(D3DRS_CLIPPING, TRUE);
		m_device->SetRenderState(D3DRS_CLIPPLANEENABLE, FALSE);
		m_device->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
		m_device->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
		m_device->SetRenderState(D3DRS_FOGENABLE, FALSE);
		m_device->SetRenderState(D3DRS_COLORWRITEENABLE,
			D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN |
			D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);

		m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		m_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		m_device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		m_device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
		m_device->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
		m_device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		m_device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		m_device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		m_device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		m_device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

		m_device->SetFVF(vertex_definition);
		m_device->SetTexture(0, nullptr);
		m_device->SetStreamSource(0, m_vertex_buffer, 0, sizeof(vertex_t));
		m_device->SetPixelShader(nullptr);

        if (i != 0) {
            m_device->EndStateBlock(&m_prev_state_block);
        } else {
            m_device->EndStateBlock(&m_render_state_block);
        }
    }
}

void render::release()
{
    if (m_vertex_buffer) {
        m_vertex_buffer->Release();
        m_vertex_buffer = nullptr;
    }

    if (m_prev_state_block) {
        m_prev_state_block->Release();
        m_prev_state_block = nullptr;
    }

    if (m_render_state_block) {
        m_render_state_block->Release();
        m_render_state_block = nullptr;
    }
}

void render::begin()
{
    m_prev_state_block->Capture();
    m_render_state_block->Apply();
}

void render::end()
{
    m_prev_state_block->Apply();
}

void render::draw()
{
    draw(m_render_queue);
    m_render_queue->clear();
}

void render::draw(render_list *&render_queue)
{
    size_t num_vertices = std::size(render_queue->m_vertices);
    if (num_vertices > 0) {
		void *data;

		if (num_vertices > m_max_vertices)
		{
			m_max_vertices = num_vertices;
			release();
			reacquire();
		}

		if(FAILED(m_vertex_buffer->Lock(0, 0, &data, D3DLOCK_DISCARD))) {
			MessageBoxA(NULL, "failed to lock vertex buffer", "renderer::draw", MB_OK);
            return;
		} else {
            std::memcpy(data, std::data(render_queue->m_vertices), sizeof(vertex_t) * num_vertices);
        }
		m_vertex_buffer->Unlock();
    }

    size_t pos = 0;

    for (const auto &batch : render_queue->m_batches) {
        int order = type_order(batch.m_type);

        if (batch.m_count && order > 0) {
            std::uint32_t primitive_count = batch.m_count;

			if (is_type_list(batch.m_type))
				primitive_count /= order;
			else
				primitive_count -= (order - 1);

			m_device->SetTexture(0, batch.m_texture);
			m_device->DrawPrimitive(batch.m_type, pos, primitive_count);
			pos += batch.m_count;
        }
    }
}

render_list *render::make_render_list()
{
    return new render_list(m_max_vertices);
}

font_handle_t render::create_font(const std::string& family, long size, uint8_t flags)
{
    m_fonts.push_back(new font(this, m_device, family, size, flags));
    return font_handle_t(m_fonts.size() - 1);
}

void render::rect(render_list *&render_queue, float x, float y, float z, float w, D3DCOLOR color)
{
    vertex_t v[]
	{
		{ x, y, color },
		{ x + z, y, color },
		{ x, y + w, color },

		{ x + z, y, color },
		{ x + z, y + w, color },
		{ x, y + w, color }
	};

	add_vertices(render_queue, v, D3DPT_TRIANGLELIST);
}

void render::rect(float x, float y, float w, float h, D3DCOLOR color)
{
    render::rect(m_render_queue, x, y, w, h, color);
}

void render::box(render_list *&render_queue, float x, float y, float z, float w, D3DCOLOR color)
{
    rect(render_queue, x, y, z, 1, color);
    rect(render_queue, x, y + w - 1, z, 1, color);
    rect(render_queue, x, y, 1, w, color);
    rect(render_queue, x + z - 1, y, 1, w, color);
}

void render::box(float x, float y, float w, float h, D3DCOLOR color)
{
    render::box(m_render_queue, x, y, w, h, color);
}

void render::get_text_extent(font_handle_t font, const std::string& text, float* w, float* h)
{
	m_fonts[font.m_id]->get_text_extent(text, w, h);
}

void render::text(render_list *&render_queue, font_handle_t font, float x, float y, const std::string &text, D3DCOLOR color, uint8_t flags)
{
    if (font.m_id < 0 || font.m_id >= std::size(m_fonts)) {
        MessageBoxA(NULL, "Bad font handle!", "render::text", MB_OK);
        return;
    }

	m_fonts[font.m_id]->draw_text(render_queue, x, y, text, color, flags);
}

void render::text(font_handle_t font, float x, float y, const std::string &text, D3DCOLOR color, uint8_t flags)
{
    render::text(m_render_queue, font, x, y, text, color, flags);
}

render_list::render_list(size_t max_vertices)
{
    m_vertices.reserve(max_vertices);
}

void render_list::clear()
{
    m_vertices.clear();
    m_batches.clear();
}

font_handle_t::font_handle_t(size_t id)
    : m_id(id)
{
}

batch_t::batch_t(size_t count, D3DPRIMITIVETYPE type, IDirect3DTexture9 *texture)
    : m_count(count), m_type(type), m_texture(texture)
{
}

vertex_t::vertex_t(float x, float y, float z, float r, D3DCOLOR color, float t_x, float t_y)
    : m_x(x), m_y(y), m_z(z), m_r(r), m_color(color), m_texture_x(t_x), m_texture_y(t_y) 
{
}

vertex_t::vertex_t(float x, float y, float z, float r, D3DCOLOR color)
    : m_x(x), m_y(y), m_z(z), m_r(r), m_color(color)
{
}

vertex_t::vertex_t(float x, float y, float z, D3DCOLOR color)
    : m_x(x), m_y(y), m_z(z), m_r(1.f), m_color(color)
{
}

vertex_t::vertex_t(float x, float y, D3DCOLOR color)
    : m_x(x), m_y(y), m_z(1.f), m_r(1.f), m_color(color)
{
}

font::font(render *renderer, IDirect3DDevice9 *device, const std::string &family, long height, uint8_t flags)
    : m_renderer(renderer), m_device(device), m_family(family), m_height(height), m_flags(flags), m_spacing(0), m_texture(nullptr)
{
    HDC gdi_ctx           = nullptr;
	HGDIOBJ gdi_font      = nullptr;
	HGDIOBJ prev_gdi_font = nullptr;
	HBITMAP bitmap        = nullptr;
	HGDIOBJ prev_bitmap   = nullptr;

	text_scale = 1.0f;

	gdi_ctx = CreateCompatibleDC(nullptr);
	SetMapMode(gdi_ctx, MM_TEXT);
	
	create_gdi_font(gdi_ctx, &gdi_font);
	
	if (!gdi_font) {
		MessageBoxA(NULL, "Failed to create GDI font!", "font", MB_OK);
        return;
	}

	prev_gdi_font = SelectObject(gdi_ctx, gdi_font);

	tex_width = tex_height = 128;

	HRESULT hr = S_OK;
	while (D3DERR_MOREDATA == (hr = paint_alphabet(gdi_ctx, true)))
	{
		tex_width *= 2;
		tex_height *= 2;
	}

	if (FAILED(hr)) {
        MessageBoxA(NULL, "Failed to paint alphabet", "font", MB_OK);
        return;
    }

	D3DCAPS9 d3dCaps;
	m_device->GetDeviceCaps(&d3dCaps);

	if (tex_width > static_cast<long>(d3dCaps.MaxTextureWidth))
	{
		text_scale = static_cast<float>(d3dCaps.MaxTextureWidth) / tex_width;
		tex_width = tex_height = d3dCaps.MaxTextureWidth;

		bool first_iteration = true;

		do
		{
			if (!first_iteration)
				text_scale *= 0.9f;

			DeleteObject(SelectObject(gdi_ctx, prev_gdi_font));

			create_gdi_font(gdi_ctx, &gdi_font);

			if (!gdi_font) {
                MessageBoxA(NULL, "Failed to create GDI font!", "font", MB_OK);
                return;
			}

			prev_gdi_font = SelectObject(gdi_ctx, gdi_font);

			first_iteration = false;
		} while (D3DERR_MOREDATA == (hr = paint_alphabet(gdi_ctx, true)));
	}

	if (FAILED(m_device->CreateTexture(tex_width, tex_height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A4R4G4B4, D3DPOOL_DEFAULT, &m_texture, nullptr))) {
        MessageBoxA(NULL, "Failed to create texture!", "font", MB_OK);
        return;
    }

	DWORD* bitmap_bits = nullptr;

	BITMAPINFO bitmap_ctx {};
	bitmap_ctx.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	bitmap_ctx.bmiHeader.biWidth       = tex_width;
	bitmap_ctx.bmiHeader.biHeight      = -tex_height;
	bitmap_ctx.bmiHeader.biPlanes      = 1;
	bitmap_ctx.bmiHeader.biCompression = BI_RGB;
	bitmap_ctx.bmiHeader.biBitCount    = 32;

	bitmap = CreateDIBSection(gdi_ctx, &bitmap_ctx, DIB_RGB_COLORS, reinterpret_cast<void**>(&bitmap_bits), nullptr, 0);

	prev_bitmap = SelectObject(gdi_ctx, bitmap);

	SetTextColor(gdi_ctx, RGB(255, 255, 255));
	SetBkColor(gdi_ctx, 0x00000000);
	SetTextAlign(gdi_ctx, TA_TOP);

	if (FAILED(paint_alphabet(gdi_ctx, false))) {
        MessageBoxA(NULL, "Failed to paint alphabet", "font", MB_OK);
        return;
    }

	D3DLOCKED_RECT locked_rect;
	m_texture->LockRect(0, &locked_rect, nullptr, 0);

	std::uint8_t *dst_row = static_cast<std::uint8_t *>(locked_rect.pBits);
	BYTE alpha;

	for (long y = 0; y < tex_height; y++)
	{
		std::uint16_t *dst = reinterpret_cast<std::uint16_t *>(dst_row);
		for (long x = 0; x < tex_width; x++)
		{
			alpha = ((bitmap_bits[tex_width*y + x] & 0xff) >> 4);
			if (alpha > 0)
			{
				*dst++ = ((alpha << 12) | 0x0fff);
			}
			else
			{
				*dst++ = 0x0000;
			}
		}
		dst_row += locked_rect.Pitch;
	}

	if (m_texture)
		m_texture->UnlockRect(0);

	SelectObject(gdi_ctx, prev_bitmap);
	SelectObject(gdi_ctx, prev_gdi_font);
	DeleteObject(bitmap);
	DeleteObject(gdi_font);
	DeleteDC(gdi_ctx);
}

font::~font()
{
    if (m_texture) {
        m_texture->Release();
        m_texture = nullptr;
    }
}

void font::draw_text(render_list *&render_queue, float x, float y, const std::string &text, D3DCOLOR color, uint8_t flags)
{
    std::size_t num_to_skip = 0;

	if (flags & (TEXT_RIGHT | TEXT_CENTERED))
	{
        float width, height;
		get_text_extent(text, &width, &height);
		
		if (flags & TEXT_RIGHT)
			x -= width;
		else if (flags & TEXT_CENTERED_X)
			x -= 0.5f * width;

		if (flags & TEXT_CENTERED_Y)
			y -= 0.5f * height;
	}

	x -= m_spacing;

	float start_x = x;

	for (const auto &c : text)
	{
		if (num_to_skip > 0 && num_to_skip-- > 0)
			continue;

		if (flags & TEXT_COLORTAGS && c == '{') // format: {#aarrggbb} or {##rrggbb}, {#aarrggbb} will inherit alpha from color argument.
		{
			std::size_t index = &c - &text[0];
			if (std::size(text) > index + 11)
			{
				std::string color_str = text.substr(index, 11);
				if (color_str[1] == '#')
				{
					bool alpha = false;
					if ((alpha = color_str[10] == '}') || color_str[8] == '}')
					{
						num_to_skip += alpha ? 10 : 8;
						color_str.erase(std::remove_if(std::begin(color_str), std::end(color_str), [](char c) { return !std::isalnum(c); }), std::end(color_str));
						color = std::stoul(alpha ? color_str : "ff" + color_str, nullptr, 16);
						continue;
					}
				}
			}
		}

		if (c == '\n')
		{
			x = start_x;
			y += (tex_coords[0][3] - tex_coords[0][1]) * tex_height;
		}

		if (c < ' ')
			continue;

		float tx1 = tex_coords[c - 32][0];
		float ty1 = tex_coords[c - 32][1];
		float tx2 = tex_coords[c - 32][2];
		float ty2 = tex_coords[c - 32][3];

		float w = (tx2 - tx1) * tex_width / text_scale;
		float h = (ty2 - ty1) * tex_height / text_scale;

		if (c != ' ')
		{
			vertex_t v[] =
			{
				{ x - 0.5f,     y - 0.5f + h, 0.9f, 1.f , color, tx1, ty2 },
				{ x - 0.5f,     y - 0.5f,     0.9f, 1.f , color, tx1, ty1 },
				{ x - 0.5f + w, y - 0.5f + h, 0.9f, 1.f , color, tx2, ty2 },

				{ x - 0.5f + w, y - 0.5f,     0.9f, 1.f , color, tx2, ty1 },
				{ x - 0.5f + w, y - 0.5f + h, 0.9f, 1.f , color, tx2, ty2 },
				{ x - 0.5f,     y - 0.5f,     0.9f, 1.f , color, tx1, ty1 }
			};

			if (flags & TEXT_SHADOW)
			{
				D3DCOLOR shadow_color = D3DCOLOR_ARGB((color >> 24) & 0xff, 0x00, 0x00, 0x00);

				for (auto &vtx : v) { vtx.m_color = shadow_color; vtx.m_x += 1.f; }
				m_renderer->add_vertices(render_queue, v, D3DPT_TRIANGLELIST, m_texture);

				for (auto &vtx : v) { vtx.m_x -= 2.f; }
				m_renderer->add_vertices(render_queue, v, D3DPT_TRIANGLELIST, m_texture);

				for (auto &vtx : v) { vtx.m_x += 1.f; vtx.m_y += 1.f; }
				m_renderer->add_vertices(render_queue, v, D3DPT_TRIANGLELIST, m_texture);

				for (auto &vtx : v) { vtx.m_y -= 2.f; }
				m_renderer->add_vertices(render_queue, v, D3DPT_TRIANGLELIST, m_texture);
		
				for (auto &vtx : v) { vtx.m_color = color; vtx.m_y -= 1.f; }
			}

			m_renderer->add_vertices(render_queue, v, D3DPT_TRIANGLELIST, m_texture);
		}

		x += w - (2.f * m_spacing);
	}
}

void font::get_text_extent(const std::string &text, float *w, float *h)
{
    float row_width = 0.f;
	float row_height = (tex_coords[0][3] - tex_coords[0][1])* tex_height;
	float width = 0.f;
	float height = row_height;

	for (const auto &c : text)
	{
		if (c == '\n')
		{
			row_width = 0.f;
			height += row_height;
		}

		if (c < ' ')
			continue;

		float tx1 = tex_coords[c - 32][0];
		float tx2 = tex_coords[c - 32][2];

		row_width += (tx2 - tx1) * tex_width - 2.f * m_spacing;

		if (row_width > width)
			width = row_width;
	}

	*w = width;
    *h = height;
}

void font::create_gdi_font(HDC ctx, HGDIOBJ *gdi_font)
{
    int character_height = -MulDiv(m_height, static_cast<int>(GetDeviceCaps(ctx, LOGPIXELSY) * text_scale), 72);

	DWORD bold = (m_flags & FONT_BOLD) ? FW_BOLD : FW_NORMAL;
	DWORD italic = (m_flags & FONT_ITALIC) ? TRUE : FALSE;
	
	*gdi_font = CreateFontA(character_height, 0, 0, 0, bold, italic, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH, m_family.c_str());
}

HRESULT font::paint_alphabet(HDC ctx, bool measure_only)
{
	SIZE size;
	char chr[2] = "x";

	if (0 == GetTextExtentPoint32A(ctx, chr, 1, &size))
		return E_FAIL;

	m_spacing = static_cast<long>(ceil(size.cy * 0.3f));

	long x = m_spacing;
	long y = 0;

	for (char c = 32; c < 127; c++)
	{
		chr[0] = c;
		if (0 == GetTextExtentPoint32A(ctx, chr, 1, &size))
			return E_FAIL;

		if (x + size.cx + m_spacing > tex_width)
		{
			x = m_spacing;
			y += size.cy + 1;
		}

		if (y + size.cy > tex_height)
			return D3DERR_MOREDATA;

		if (!measure_only)
		{
			if (0 == ExtTextOutA(ctx, x + 0, y + 0, ETO_OPAQUE, nullptr, chr, 1, nullptr))
				return E_FAIL;

			tex_coords[c - 32][0] = (static_cast<float>(x + 0 - m_spacing))       / tex_width;
			tex_coords[c - 32][1] = (static_cast<float>(y + 0 + 0))             / tex_height;
			tex_coords[c - 32][2] = (static_cast<float>(x + size.cx + m_spacing)) / tex_width;
			tex_coords[c - 32][3] = (static_cast<float>(y + size.cy + 0))       / tex_height;
		}

		x += size.cx + (2 * m_spacing);
	}

	return S_OK;
}
