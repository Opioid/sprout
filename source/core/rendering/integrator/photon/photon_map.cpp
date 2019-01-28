#include "photon_map.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/thread/thread_pool.hpp"

#include <iostream>
#include "base/math/print.hpp"

namespace rendering::integrator::photon {

Map::Map(uint32_t num_photons, float search_radius, float merge_radius,
         float indirect_radius_factor, bool separate_indirect) noexcept
    : num_photons_(num_photons),
      photons_(nullptr),
      separate_indirect_(separate_indirect),
      num_reduced_(nullptr),
      caustic_grid_(search_radius, merge_radius, 1.75f),
      indirect_grid_(indirect_radius_factor * search_radius, merge_radius, 1.1f) {}

Map::~Map() noexcept {
    memory::free_aligned(num_reduced_);
    memory::free_aligned(photons_);
}

void Map::init(uint32_t num_workers) noexcept {
    photons_     = memory::allocate_aligned<Photon>(num_photons_);
    num_reduced_ = memory::allocate_aligned<uint32_t>(num_workers);
}

void Map::insert(Photon const& photon, uint32_t index) noexcept {
    photons_[index] = photon;
}

uint32_t Map::compile_iteration(uint32_t num_paths, thread::Pool& pool) noexcept {
    AABB const aabb = calculate_aabb(pool);

    caustic_grid_.resize(aabb);

    num_paths_ = num_paths;

    if (separate_indirect_) {
        indirect_grid_.resize(aabb);

        auto const indirect_photons = std::partition(
            photons_, photons_ + num_photons_,
            [](Photon const& p) { return p.properties.test(Photon::Property::First_hit); });

        uint32_t const num_caustics = static_cast<uint32_t>(
            std::distance(photons_, indirect_photons));

        uint32_t const num_indirect = num_photons_ - num_caustics;

        caustic_grid_.set_range(num_caustics, photons_);

        uint32_t const red_num_caustics = caustic_grid_.reduce_and_move(photons_, num_reduced_,
                                                                        pool);

        indirect_grid_.set_range(num_indirect, photons_ + num_caustics);

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
        caustic_grid_.set_range(num_photons_, photons_);

        uint32_t const red_num_caustics = caustic_grid_.reduce_and_move(photons_, num_reduced_,
                                                                        pool);

        float const percentage_caustics = static_cast<float>(red_num_caustics) /
                                          static_cast<float>(num_photons_);

        std::cout << red_num_caustics << " total left of " << num_photons_ << " ("
                  << static_cast<uint32_t>(100.f * percentage_caustics) << "%)" << std::endl;

        return red_num_caustics;
    }
}

void Map::compile_finalize() noexcept {
    caustic_grid_.init_cells();

    if (separate_indirect_) {
        indirect_grid_.init_cells();
    }
}

float3 Map::li(Intersection const& intersection, Material_sample const& sample,
               scene::Worker const& worker) const noexcept {
    return caustic_grid_.li(intersection, sample, num_paths_, worker) +
           indirect_grid_.li(intersection, sample, num_paths_, worker);
}

size_t Map::num_bytes() const noexcept {
    size_t num_bytes = num_photons_ * sizeof(Photon);

    num_bytes += caustic_grid_.num_bytes() + indirect_grid_.num_bytes();

    return num_bytes;
}

AABB Map::calculate_aabb(thread::Pool& /*pool*/) const noexcept {
    AABB aabb = AABB::empty();

    for (uint32_t i = 0, len = num_photons_; i < len; ++i) {
        aabb.insert(photons_[i].p);
    }

    aabb.add(0.0001f);

    return aabb;
}

}  // namespace rendering::integrator::photon
