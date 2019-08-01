#include "item.hpp"
#include "core/image/texture/texture.inl"

bool any_has_alpha_channel(std::vector<Item> const& items) noexcept {
    for (auto const& i : items) {
        if (4 == i.image->num_channels()) {
            return true;
        }
    }

    return false;
}
