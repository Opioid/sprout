#include "photon_map.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/thread/thread_pool.hpp"

#include <iostream>
#include "base/math/print.hpp"

namespace rendering::integrator::photon {

Map::Map(uint32_t num_photons, float radius, float indirect_radius_factor, bool separate_caustics)
    : num_photons_(num_photons),
      photons_(nullptr),
      radius_(radius),
      indirect_radius_factor_(indirect_radius_factor),
      separate_caustics_(separate_caustics),
      num_reduced_(nullptr) {}

Map::~Map() {
    memory::free_aligned(num_reduced_);
    memory::free_aligned(photons_);
}

void Map::init(uint32_t num_workers) {
    photons_     = memory::allocate_aligned<Photon>(num_photons_);
    num_reduced_ = memory::allocate_aligned<uint32_t>(num_workers);
}

void Map::resize(math::AABB const& aabb) {
    float constexpr merge_threshold = 0.15f;

    caustic_grid_.resize(aabb, radius_, merge_threshold);

    if (separate_caustics_) {
        indirect_grid_.resize(aabb, indirect_radius_factor_ * radius_,
                              merge_threshold / indirect_radius_factor_);
    }
}

void Map::insert(Photon const& photon, uint32_t index) {
    photons_[index] = photon;
}

uint32_t Map::compile(uint32_t num_paths, thread::Pool& pool) {
    num_paths_ = num_paths;

    if (separate_caustics_) {
        auto const indirect_photons = std::partition(
            photons_, photons_ + num_photons_,
            [](Photon const& p) { return p.properties.test(Photon::Property::First_hit); });

        uint32_t const num_caustics = static_cast<uint32_t>(
            std::distance(photons_, indirect_photons));
        uint32_t const num_indirect = num_photons_ - num_caustics;

        caustic_grid_.update(num_caustics, photons_);
        indirect_grid_.update(num_indirect, photons_ + num_caustics);

        uint32_t const red_num_caustics = caustic_grid_.reduce_and_move(photons_, num_reduced_,
                                                                        pool);
        uint32_t const red_num_indirect = indirect_grid_.reduce_and_move(
            photons_ + red_num_caustics, num_reduced_, pool);

        float const percentage_caustics = static_cast<float>(red_num_caustics) /
                                          static_cast<float>(num_caustics);

        float const percentage_indirect = static_cast<float>(red_num_indirect) /
                                          static_cast<float>(num_indirect);

        std::cout << red_num_caustics << " caustics left of " << num_caustics << " ("
                  << static_cast<uint32_t>(100.f * percentage_caustics) << "%)" << std::endl;
        std::cout << red_num_indirect << " indirect left of " << num_indirect << " ("
                  << static_cast<uint32_t>(100.f * percentage_indirect) << "%)" << std::endl;

        return red_num_caustics + red_num_indirect;
    } else {
        caustic_grid_.update(num_photons_, photons_);

        uint32_t const red_num_caustics = caustic_grid_.reduce_and_move(photons_, num_reduced_,
                                                                        pool);

        float const percentage_caustics = static_cast<float>(red_num_caustics) /
                                          static_cast<float>(num_photons_);

        std::cout << red_num_caustics << " total left of " << num_photons_ << " ("
                  << static_cast<uint32_t>(100.f * percentage_caustics) << "%)" << std::endl;

        return red_num_caustics;
    }
}

float3 Map::li(scene::prop::Intersection const& intersection, scene::material::Sample const& sample,
               scene::Worker const& worker) const {
    return caustic_grid_.li(intersection, sample, num_paths_, worker) +
           indirect_grid_.li(intersection, sample, num_paths_, worker);
}

size_t Map::num_bytes() const {
    size_t num_bytes = num_photons_ * sizeof(Photon);

    num_bytes += caustic_grid_.num_bytes() + indirect_grid_.num_bytes();

    return num_bytes;
}

}  // namespace rendering::integrator::photon
