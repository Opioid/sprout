#ifndef SU_IT_ITEM_HPP
#define SU_IT_ITEM_HPP

#include <string>
#include <vector>

namespace image::texture {
class Texture;
}

using namespace image;

using Texture = texture::Texture;

struct Item {
    std::string name;
    std::string name_out;

    Texture const* image;
};

bool any_has_alpha_channel(std::vector<Item> const& items) noexcept;

#endif
