#ifndef SU_IT_ITEM_HPP
#define SU_IT_ITEM_HPP

#include "base/math/vector2.hpp"

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

bool any_has_alpha_channel(std::vector<Item> const& items);

int2 max_dimensions_2(std::vector<Item> const& items);

std::string name_out(std::vector<Item> const& items, std::string const& default_name);

#endif
