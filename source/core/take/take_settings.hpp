#ifndef SU_CORE_TAKE_SETTINGS_HPP
#define SU_CORE_TAKE_SETTINGS_HPP

#include <cstdint>

namespace take {

struct Settings {
    float ray_offset_factor = 1.f;
};

struct Photon_settings {
    uint32_t num_photons          = 0;
    uint32_t max_photons_per_path = 1;
    uint32_t max_bounces          = 2;
    float    radius               = 0.05f;
    bool     indirect_caustics    = false;
};

}  // namespace take

#endif
