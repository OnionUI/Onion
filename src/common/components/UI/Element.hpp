#ifndef UI_ELEMENT_HPP__
#define UI_ELEMENT_HPP__

#include <iostream>
#include <list>
#include <SDL/SDL.h>

#include "Screen.hpp"
#include "Surface.hpp"

using std::list;
using namespace UI;

namespace UI {

class Element : Surface
{
public:
    Element *parent;
    list<Element*> children;

    Element(signed short x, signed short y, unsigned short w, unsigned short h) : Surface(NULL) {
        std::cout << "Create element {" << x << ", " << y << ", " << w << ", " << h << "}" << std::endl;
        parent = NULL;
    }

    void render(Surface *dst) {
        for (auto child : children)
            child->render((Surface*)this);

        blit(dst);
    }
    void render(Element *dst) { render((Surface*)dst); }
    void render(Screen *dst) { render((Surface*)dst); }

    void appendChild(UI::Element *child) {
        child->parent = this;
        children.push_back(child);
    }
};

}

#endif // UI_ELEMENT_HPP__
