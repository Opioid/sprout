#include "photon_map.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/thread/thread_pool.hpp"

#include <iostream>
#include "base/math/print.hpp"

namespace rendering::integrator::photon {

Map::Map(uint32_t num_photons, float radius, float indirect_radius_factor, bool separate_caustics,
         uint32_t num_workers)
    : num_photons_(num_photons),
      photons_(nullptr),
      radius_(radius),
      indirect_radius_factor_(indirect_radius_factor),
      separate_caustics_(separate_caustics),
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

    caustic_grid_.resize(aabb, radius_);

    if (separate_caustics_) {
        indirect_grid_.resize(aabb, indirect_radius_factor_ * radius_);

        auto const indirect_photons = std::partition(photons_, photons_ + num_photons_,
                                                     [](Photon const& p) { return p.caustic; });

        uint32_t const num_caustics = std::distance(photons_, indirect_photons);
        uint32_t const num_indirect = num_photons_ - num_caustics;

        caustic_grid_.update(num_caustics, photons_, true);
        indirect_grid_.update(num_indirect, photons_ + num_caustics, true);

        uint32_t const comp_num_caustics = caustic_grid_.reduce(num_reduced_, pool);
        uint32_t const comp_num_indirect = indirect_grid_.reduce(num_reduced_, pool);

        float const percentage_caustics = static_cast<float>(comp_num_caustics) /
                                          static_cast<float>(num_caustics);

        float const percentage_indirect = static_cast<float>(comp_num_indirect) /
                                          static_cast<float>(num_indirect);

        std::cout << comp_num_caustics << " caustics left of " << num_caustics << " ("
                  << static_cast<uint32_t>(100.f * percentage_caustics) << "%)" << std::endl;
        std::cout << comp_num_indirect << " indirect left of " << num_indirect << " ("
                  << static_cast<uint32_t>(100.f * percentage_indirect) << "%)" << std::endl;

        uint32_t const comp_num_photons = comp_num_caustics + comp_num_indirect;

        Photon* comp_photons = new Photon[comp_num_photons];

        for (uint32_t i = 0, j = 0, len = num_caustics; i < len; ++i) {
            if (photons_[i].alpha[0] >= 0.f) {
                comp_photons[j++] = photons_[i];
            }
        }

        for (uint32_t i = num_caustics, j = comp_num_caustics, len = num_photons_; i < len; ++i) {
            if (photons_[i].alpha[0] >= 0.f) {
                comp_photons[j++] = photons_[i];
            }
        }

        delete[] photons_;
        photons_     = comp_photons;
        num_photons_ = comp_num_photons;

        caustic_grid_.update(comp_num_caustics, photons_, false);
        indirect_grid_.update(comp_num_indirect, photons_ + comp_num_caustics, false);
    } else {
        caustic_grid_.update(num_photons_, photons_, true);

        uint32_t const comp_num_caustics = caustic_grid_.reduce(num_reduced_, pool);

        float const percentage_caustics = static_cast<float>(comp_num_caustics) /
                                          static_cast<float>(num_photons_);

        std::cout << comp_num_caustics << " total left of " << num_photons_ << " ("
                  << static_cast<uint32_t>(100.f * percentage_caustics) << "%)" << std::endl;

        uint32_t const comp_num_photons = comp_num_caustics;

        Photon* comp_photons = new Photon[comp_num_photons];

        for (uint32_t i = 0, j = 0, len = num_photons_; i < len; ++i) {
            if (photons_[i].alpha[0] >= 0.f) {
                comp_photons[j++] = photons_[i];
            }
        }

        delete[] photons_;
        photons_     = comp_photons;
        num_photons_ = comp_num_photons;

        caustic_grid_.update(comp_num_photons, photons_, false);
    }
}

float3 Map::li(f_float3 position, scene::material::Sample const& sample) const {
    return caustic_grid_.li(position, sample, num_paths_) +
           indirect_grid_.li(position, sample, num_paths_);
}

size_t Map::num_bytes() const {
    size_t num_bytes = num_photons_ * sizeof(Photon);

    num_bytes += caustic_grid_.num_bytes() + indirect_grid_.num_bytes();

    return num_bytes;
}

}  // namespace rendering::integrator::photon
