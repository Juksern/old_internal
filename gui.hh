#pragma once
#include "includes.hh"

class render;

enum elements_e : uint8_t {
    ELEMENT_CHECKBOX = 0 << 0,
    ELEMENT_SLIDER = 1 << 1,
    ELEMENT_LIST = 1 << 2
};

class element {
public:
    element() = default;
    ~element() = default;

    element(int* var, uint8_t type = ELEMENT_CHECKBOX);
    element(int* var, int var_min, int var_max, uint8_t type = ELEMENT_SLIDER);
    element(int* var, const std::array<int, 5> items, uint8_t type = ELEMENT_LIST);

    void draw(render* &renderer);
    void handle();

    float m_x, m_y;
    int m_width, m_height;
private:
    std::uint8_t m_type;
    int *m_cur, m_min, m_max;
    std::array<int, 5> m_items; 
};

class group {
public:
    group(const std::string& title);
    ~group();

    void draw(render* &renderer);
    void handle();
    void add(element* e);

    float m_x, m_y;
    int m_width, m_height;
private:
    std::string m_title;
    std::vector<element*> m_elements;
};

class tab {
public:
    tab(const std::string& title);
    ~tab();

    void draw(render* renderer);
    void handle();
    void add(group* e);

    float m_x, m_y;
    int m_width, m_height;
private:
    std::string m_title;
    std::vector<group*> m_groups;
};

class gui {
public:
    gui(const std::string& title, float x, float y);
    ~gui();

    void draw(render* renderer);
    void handle();
    void add(tab* e);
    void initialize();

private:
    float m_x, m_y;
    int m_width, m_height;
    std::string m_title;
    std::vector<tab*> m_tabs;
};

extern gui *gGui;