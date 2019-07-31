#ifndef SU_IT_ITEM_HPP
#define SU_IT_ITEM_HPP

#include <string>

namespace image::texture {
class Texture;
}

using namespace image;

using Texture = texture::Texture;

struct Item {
    std::string name;

    Texture const* image;
};

#endif
