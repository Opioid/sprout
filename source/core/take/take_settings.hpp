#ifndef SU_CORE_TAKE_SETTINGS_HPP
#define SU_CORE_TAKE_SETTINGS_HPP

#include <cstdint>

namespace take {

struct Settings {
    float ray_offset_factor = 1.f;
};

struct Photon_settings {
    uint32_t num_photons = 0;
    float    radius      = 0.05f;
};

}  // namespace take

#endif
