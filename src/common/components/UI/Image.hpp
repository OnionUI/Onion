#ifndef UI_IMAGE_HPP__
#define UI_IMAGE_HPP__

#include <string>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "Element.hpp"

using std::string;
using namespace UI;

namespace UI {

class Image : Surface
{
public:
    Image(string path) : Surface(IMG_Load(path.c_str())) {};
};

}

#endif // UI_IMAGE_HPP__
