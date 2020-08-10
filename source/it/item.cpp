#include "item.hpp"
#include "base/string/string.hpp"
#include "core/image/texture/texture.inl"

bool any_has_alpha_channel(std::vector<Item> const& items) {
    for (auto const& i : items) {
        if (4 == i.image.num_channels()) {
            return true;
        }
    }

    return false;
}

int2 max_dimensions_2(std::vector<Item> const& items) {
    int2 dm(0);

    for (auto const& i : items) {
        dm = max(dm, i.image.dimensions().xy());
    }

    return dm;
}

std::string name_out(std::vector<Item> const& items, std::string const& default_name) {
    auto const& i = items[0];

    return i.name_out.empty() ? (default_name + "." + string::copy_suffix(i.name)) : i.name_out;
}
