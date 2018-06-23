#include "photon_map.hpp"
#include "base/math/vector3.inl"

namespace rendering::integrator::photon {

Map::Map(uint32_t num_photons) : num_photons_(num_photons), photons_(new Photon[num_photons]) {}

Map::~Map() {
    delete[] photons_;
}

void Map::insert(Photon const& photon, int32_t index) {
    photons_[index] = photon;
}

float3 Map::li(f_float3 position, scene::material::Sample const& sample) const {
    float const radius = 0.01f;

    for (uint32_t i = 0, len = num_photons_; i < len; ++i) {
        if (math::distance(photons_[i].p, position) <= radius) {
            return float3(1.f);
        }
    }

    return float3::identity();
}

}  // namespace rendering::integrator::photon
