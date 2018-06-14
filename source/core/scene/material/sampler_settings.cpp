#include "sampler_settings.hpp"

namespace scene::material {

Sampler_settings::Sampler_settings(Filter filter, Address address_u, Address address_v)
    : filter(filter), address_u(address_u), address_v(address_v) {}

uint32_t Sampler_settings::key() const {
    uint32_t const address_flat = static_cast<uint32_t>(address_u) << 1 |
                                  static_cast<uint32_t>(address_v);

    uint32_t const key = static_cast<uint32_t>(filter) | address_flat;
    return key;
}

}  // namespace scene::material
