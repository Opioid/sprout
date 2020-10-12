#ifndef SU_SCENE_MATERIAL_SAMPLER_SETTINGS_HPP
#define SU_SCENE_MATERIAL_SAMPLER_SETTINGS_HPP

#include <cstdint>

namespace scene::material {

struct Sampler_settings {
    enum class Address : uint8_t {
        Clamp     = 0 << 0,
        Repeat    = 1 << 0,
        Mask      = 0b00000011,
        Undefined = 0xFF
    };

    enum class Filter : uint8_t { Nearest = 0 << 2, Linear = 1 << 2, Undefined = 0xFF };

    Sampler_settings(Filter filter = Filter::Linear, Address address_u = Address::Repeat,
                     Address address_v = Address::Repeat);

    uint32_t key() const;

    Filter filter;

    Address address_u;
    Address address_v;
};

}  // namespace scene::material

#endif
