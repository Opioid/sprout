#ifndef SU_CORE_TAKE_SETTINGS_HPP
#define SU_CORE_TAKE_SETTINGS_HPP

#include <cstdint>

namespace take {

struct Settings {
    float ray_offset_factor = 1.f;
};

struct Photon_settings {
    uint32_t num_photons = 0;
    uint32_t max_bounces = 2;

    float iteration_threshold    = 0.f;
    float radius                 = 0.05f;
    float indirect_radius_factor = 4.f;

    bool indirect_caustics = false;
    bool separate_caustics = true;
    bool full_light_path;
};

}  // namespace take

#endif
