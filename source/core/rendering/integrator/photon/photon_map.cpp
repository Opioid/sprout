#include "photon_map.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/thread/thread_pool.hpp"

#include <iostream>
#include "base/math/print.hpp"

namespace rendering::integrator::photon {

Map::Map(uint32_t num_photons, float photon_radius, uint32_t num_workers)
    : num_photons_(num_photons),
      photons_(nullptr),
      photon_radius_(photon_radius),
      num_reduced_(new uint32_t[num_workers]) {}

Map::~Map() {
    delete[] num_reduced_;
    delete[] photons_;
}

void Map::prepare() {
    delete[] photons_;
    photons_ = new Photon[num_photons_];
}

void Map::insert(Photon const& photon, uint32_t index) {
    photons_[index] = photon;
}

void Map::compile(uint32_t num_paths, math::AABB const& aabb, thread::Pool& pool) {
    num_paths_ = num_paths;

    aabb_ = aabb;

    caustic_grid_.resize(aabb, photon_radius_);

    caustic_grid_.update(num_photons_, photons_, true);

    uint32_t const comp_num_photons = caustic_grid_.reduce(num_reduced_, pool);

    float const percentage = static_cast<float>(comp_num_photons) /
                             static_cast<float>(num_photons_);

    std::cout << comp_num_photons << " left of " << num_photons_ << " ("
              << static_cast<uint32_t>(100.f * percentage) << "%)" << std::endl;

    Photon* comp_photons = new Photon[comp_num_photons];

    for (uint32_t i = 0, j = 0, len = num_photons_; i < len; ++i) {
        if (photons_[i].alpha[0] >= 0.f) {
            comp_photons[j++] = photons_[i];
        }
    }

    delete[] photons_;
    photons_     = comp_photons;
    num_photons_ = comp_num_photons;

    caustic_grid_.update(num_photons_, photons_, false);
}

float3 Map::li(f_float3 position, scene::material::Sample const& sample) const {
    return caustic_grid_.li(position, sample, num_paths_);
}

size_t Map::num_bytes() const {
    size_t num_bytes = num_photons_ * sizeof(Photon);

    num_bytes += caustic_grid_.num_bytes();

    return num_bytes;
}

}  // namespace rendering::integrator::photon
