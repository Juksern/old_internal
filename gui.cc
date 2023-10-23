#include "gui.hh"

extern font_handle_t Font;
tab* current_tab = nullptr;

// checkbox
element::element(int *var, uint8_t type)
    : m_cur(var), m_type(type), m_min(0), m_max(0), m_x(0.f), m_y(0.f)
{
    m_height = 25;
    m_width = 25;
}

//slider
element::element(int *var, int var_min, int var_max, uint8_t type)
    : m_cur(var), m_type(type), m_min(var_min), m_max(var_max), m_x(0.f), m_y(0.f)
{
    m_height = 25;
    m_width = 25;
}

//list
element::element(int *var, const std::array<int, 5> items, uint8_t type)
    : m_cur(var), m_type(type), m_min(0), m_max(5), m_x(0.f), m_y(0.f), m_items(items)
{
    m_height = 25;
    m_width = 25;
}

void element::draw(render *&renderer)
{
    if (m_type == ELEMENT_CHECKBOX) {

    }

    if (m_type == ELEMENT_SLIDER) {

    }

    if (m_type == ELEMENT_LIST) {

    }
}

void element::handle()
{
    if (m_type == ELEMENT_CHECKBOX) {

    }

    if (m_type == ELEMENT_SLIDER) {

    }

    if (m_type == ELEMENT_LIST) {
        
    }
}

group::group(const std::string &title)
    : m_title(title), m_x(0.f), m_y(0.f)
{
    m_height = 100;
    m_width = 100;
}

group::~group()
{
    m_elements.clear();
}

void group::draw(render *&renderer)
{
    //draw group

    //draw children
    for (size_t i = 0; i < m_elements.size(); i++) {
        auto cur = m_elements.at(i);

        cur->draw(renderer);
    }
}

void group::handle()
{
    for (auto& e : m_elements)
        e->handle();
}

void group::add(element *e)
{
    m_elements.push_back(e);
}

tab::tab(const std::string &title)
    : m_title(title), m_x(0.f), m_y(0.f)
{
    m_height = 100;
    m_width = 100;
}

tab::~tab()
{
    for (auto& g : m_groups) {
        delete g;
    }

    m_groups.clear();
}

void tab::draw(render *renderer)
{
    //draw tab
    float text_w, text_h;
    renderer->get_text_extent(Font, m_title, &text_w, &text_h);
    m_width = (int)text_w + 15.f;
    m_height = (int)text_h + 2.f;

    //border
    renderer->box(m_x, m_y, m_width, m_height, D3DCOLOR_RGBA(228, 228, 228, 255));

    //body
    renderer->rect(m_x + 1.f, m_y + 1.f, m_width - 2.f, m_height, this == current_tab ? D3DCOLOR_RGBA(255, 255, 255, 255) : D3DCOLOR_RGBA(240, 240, 240, 255));

    //title
    renderer->text(Font, m_x + 7.5f, m_y + 2.f, m_title, D3DCOLOR_RGBA(0, 0, 0, 255));

    //draw groups
    for (size_t i = 0; i < m_groups.size(); i++) {
        auto cur = m_groups.at(i);

        cur->draw(renderer);
    }
}

void tab::handle()
{
    //handle tab

    //handle groups
    for (auto& g : m_groups)
        g->handle();
}

void tab::add(group *e)
{
    m_groups.push_back(e);
}

gui::gui(const std::string &title, float x, float y)
    : m_title(title), m_x(x), m_y(y)
{
    m_height = 300;
    m_width = 265;
}

gui::~gui()
{
    for (auto& t : m_tabs) {
        delete t;
    }

    m_tabs.clear();
}

void gui::draw(render *renderer)
{
    //draw gui
    //background
    renderer->rect(m_x, m_y, m_width, m_height, D3DCOLOR_RGBA(240, 240, 240, 255));

    //top bar
    renderer->rect(m_x, m_y, m_width, 20.f, D3DCOLOR_RGBA(255, 255, 255, 255));

    //title
    renderer->text(Font, m_x + 5.f, m_y + 4.f, m_title, D3DCOLOR_RGBA(0, 0, 0, 255));

    //inner box
    renderer->rect(m_x + 4.f, m_y + 40.f, m_width - 8.f, m_height - 40.f - 4.f, D3DCOLOR_RGBA(255, 255, 255, 255));

    //inner box border
    renderer->box(m_x + 4.f, m_y + 40.f, m_width - 8.f, m_height - 40.f - 4.f, D3DCOLOR_RGBA(228, 228, 228, 255));

    //border
    renderer->box(m_x - 1.f, m_y - 1.f, m_width + 2.f, m_height + 2.f, D3DCOLOR_RGBA(140, 140, 140, 240));

    //draw tabs
    for (size_t i = 0; i < m_tabs.size(); i++) {
        auto cur = m_tabs.at(i);
        cur->m_x = m_x + 4.f + ((i * cur->m_width));
        cur->m_y = m_y + 40.f - cur->m_height;
        cur->draw(renderer);
    }
}

void gui::handle()
{
    //handle gui

    //handle tabs
    for (auto& t : m_tabs)
        t->handle();
}

void gui::add(tab *e)
{
    m_tabs.push_back(e);
}

void gui::initialize()
{
    auto aim = new tab("aim");
    {
        
    } add(aim);
    auto esp = new tab("esp");
    {
        
    } add(esp);

    current_tab = aim;
}
