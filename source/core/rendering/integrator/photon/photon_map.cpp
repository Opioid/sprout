#include "photon_map.hpp"
#include "base/math/vector3.inl"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"

namespace rendering::integrator::photon {

Map::Map(uint32_t num_photons) : num_photons_(num_photons), photons_(new Photon[num_photons]) {}

Map::~Map() {
    delete[] photons_;
}

void Map::set_num_paths(uint32_t num_paths) {
    num_paths_ = num_paths;
}

void Map::insert(Photon const& photon, int32_t index) {
    photons_[index] = photon;
}

static inline float kernel(float squared_distance, float inv_squared_radius) {
    float const s = 1.f - squared_distance * inv_squared_radius;
    return (3.f * math::Pi_inv) * (s * s);
}

float3 Map::li(f_float3 position, scene::material::Sample const& sample) const {
    float const radius             = 0.025f;
    float const squared_radius     = radius * radius;
    float const inv_squared_radius = 1.f / squared_radius;

    float const num_paths = static_cast<float>(num_paths_);

    float const f = 1.f / (num_paths * squared_radius);

    float3 result = float3::identity();

    for (uint32_t i = 0, len = num_photons_; i < len; ++i) {
        auto const& photon           = photons_[i];
        float const squared_distance = math::squared_distance(photon.p, position);
        if (squared_distance <= squared_radius) {
            auto const bxdf = sample.evaluate(photon.wi);

            float const n_dot_wi = sample.base_layer().abs_n_dot(photon.wi);

            if (n_dot_wi > 0.f) {
                //   float const inwi = 1.f / sample.base_layer().clamp_abs_n_dot(photon.wi);
                float const clamped_n_dot_wi = scene::material::clamp(n_dot_wi);

                float const k = kernel(squared_distance, inv_squared_radius);

                result += (k * f) / clamped_n_dot_wi * photon.alpha * bxdf.reflection;
            }
        }
    }

    return result;
}

}  // namespace rendering::integrator::photon
