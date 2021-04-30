#ifndef SU_IT_ITEM_HPP
#define SU_IT_ITEM_HPP

#include "core/image/texture/texture.hpp"

#include <string>
#include <vector>

using Scene = scene::Scene;

using namespace image;

using Texture = texture::Texture;

struct Item {
    std::string name;
    std::string name_out;

    Texture image;
};

bool any_has_alpha_channel(std::vector<Item> const& items);

int2 max_dimensions_2(std::vector<Item> const& items, Scene const& scene);

std::string name_out(std::vector<Item> const& items, std::string const& default_name);

#endif
