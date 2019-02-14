#ifndef SU_CORE_TAKE_SETTINGS_HPP
#define SU_CORE_TAKE_SETTINGS_HPP

#include <cstdint>

namespace take {

struct Settings {};

struct Photon_settings {
    uint32_t num_photons = 0;
    uint32_t max_bounces = 2;

    float iteration_threshold  = 0.f;
    float search_radius        = 0.01f;
    float merge_radius         = 0.0025f;
    float coarse_search_radius = 0.1f;

    bool indirect_photons  = false;
    bool separate_indirect = false;
    bool full_light_path   = false;
};

}  // namespace take

#endif
