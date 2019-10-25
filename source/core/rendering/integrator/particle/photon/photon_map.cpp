#include "photon_map.hpp"
#include <string>
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/thread/thread_pool.hpp"
#include "photon.hpp"
#include "rendering/integrator/particle/particle_importance.hpp"
#include "scene/light/light.hpp"
#include "scene/scene.hpp"
#include "scene/scene_worker.hpp"

#include <iostream>
#include "base/math/print.hpp"

namespace rendering::integrator::particle::photon {

Map::Map(uint32_t num_photons, float search_radius, float merge_radius, float coarse_search_radius,
         bool separate_indirect) noexcept
    : num_photons_(num_photons),
      photons_(nullptr),
      separate_indirect_(separate_indirect),
      merge_radius_(merge_radius),
      aabbs_(nullptr),
      num_reduced_(nullptr),
      fine_grid_(search_radius, 1.5f, false),
      coarse_grid_(coarse_search_radius, 1.1f, true),
      photon_refs_(nullptr) {}

Map::~Map() noexcept {
    memory::free_aligned(photon_refs_);
    memory::free_aligned(num_reduced_);
    memory::free_aligned(aabbs_);
    memory::free_aligned(photons_);
}

void Map::init(uint32_t num_workers) noexcept {
    photons_     = memory::allocate_aligned<Photon>(num_photons_);
    aabbs_       = memory::allocate_aligned<AABB>(num_photons_);
    num_reduced_ = memory::allocate_aligned<uint32_t>(num_workers);
    photon_refs_ = memory::allocate_aligned<Photon_ref>(num_workers * Num_refs);
}

void Map::start() noexcept {
    red_num_fine_   = 0;
    red_num_coarse_ = 0;
    caustic_only_   = false;
}

void Map::insert(Photon const& photon, uint32_t index) noexcept {
    photons_[index] = photon;
}

uint32_t Map::compile_iteration(uint32_t num_photons, uint64_t num_paths,
                                thread::Pool& threads) noexcept {
    AABB const aabb = calculate_aabb(num_photons, threads);

    fine_grid_.resize(aabb);

    if (caustic_only_) {
        num_caustic_paths_ = num_paths;
    } else {
        num_caustic_paths_  = num_paths;
        num_indirect_paths_ = num_paths;
    }

    uint32_t num_reduced;

    if (separate_indirect_) {
        coarse_grid_.resize(aabb);

        auto const indirect_photons = std::partition(
            photons_, photons_ + num_photons_,
            [](Photon const& p) { return p.properties.is(Photon::Property::First_hit); });

        uint32_t const num_caustics = uint32_t(std::distance(photons_, indirect_photons));

        uint32_t const num_indirect = num_photons_ - num_caustics;

        fine_grid_.init_cells(num_caustics, photons_);

        uint32_t const red_num_caustics = fine_grid_.reduce_and_move(photons_, merge_radius_,
                                                                     num_reduced_, threads);

        float const percentage_caustics = float(red_num_caustics) / float(num_caustics);

        std::cout << red_num_caustics << " caustics left of " << num_caustics << " ("
                  << uint32_t(100.f * percentage_caustics) << "%)" << std::endl;

        red_num_fine_ = red_num_caustics;

        if (caustic_only_) {
            std::copy(indirect_photons, indirect_photons + num_indirect,
                      photons_ + red_num_caustics);

            return red_num_caustics + red_num_coarse_;
        }

        fine_grid_.init_cells(num_indirect, photons_ + num_caustics);

        uint32_t const red_num_indirect = fine_grid_.reduce_and_move(
            photons_ + red_num_caustics, merge_radius_, num_reduced_, threads);

        float const percentage_indirect = float(red_num_indirect) / float(num_indirect);

        std::cout << red_num_indirect << " indirect left of " << num_indirect << " ("
                  << uint32_t(100.f * percentage_indirect) << "%)" << std::endl;

        if (red_num_indirect <= red_num_coarse_) {
            caustic_only_ = true;
        }

        red_num_coarse_ = red_num_indirect;

        num_reduced = red_num_caustics + red_num_indirect;
    } else {
        fine_grid_.init_cells(num_photons, photons_);

        uint32_t const red_num_caustics = num_photons == num_photons_
                                              ? fine_grid_.reduce_and_move(photons_, merge_radius_,
                                                                           num_reduced_, threads)
                                              : num_photons;

        float const percentage_caustics = float(red_num_caustics) / float(num_photons_);

        std::cout << red_num_caustics << " total left of " << num_photons_ << " ("
                  << uint32_t(100.f * percentage_caustics) << "%)" << std::endl;

        red_num_fine_ = red_num_caustics;

        num_reduced = red_num_caustics;
    }

    return num_reduced;
}

void Map::compile_finalize() noexcept {
    fine_grid_.init_cells(red_num_fine_, photons_);
    fine_grid_.set_num_paths(num_caustic_paths_);

    if (separate_indirect_) {
        coarse_grid_.init_cells(red_num_coarse_, photons_ + red_num_fine_);
        coarse_grid_.set_num_paths(num_indirect_paths_);
    }
}

float3 Map::li(Intersection const& intersection, Material_sample const& sample,
               scene::Worker const& worker) const noexcept {
    Photon_ref* photon_refs = &photon_refs_[worker.id() * Num_refs];

    return fine_grid_.li(intersection, sample, photon_refs, worker) +
           coarse_grid_.li(intersection, sample, photon_refs, worker);
}

bool Map::caustics_only() const noexcept {
    return caustic_only_;
}

size_t Map::num_bytes() const noexcept {
    size_t num_bytes = num_photons_ * sizeof(Photon);

    num_bytes += fine_grid_.num_bytes() + coarse_grid_.num_bytes();

    return num_bytes;
}

AABB Map::calculate_aabb(uint32_t num_photons, thread::Pool& threads) const noexcept {
    threads.run_range(
        [this](uint32_t id, int32_t begin, int32_t end) {
            AABB aabb = AABB::empty();

            for (int32_t i = begin; i < end; ++i) {
                aabb.insert(photons_[i].p);
            }

            aabbs_[id] = aabb;
        },
        0, static_cast<int32_t>(num_photons));

    AABB aabb = AABB::empty();

    for (uint32_t i = 0, len = threads.num_threads(); i < len; ++i) {
        aabb.merge_assign(aabbs_[i]);
    }

    aabb.add(0.0001f);

    return aabb;
}

}  // namespace rendering::integrator::particle::photon
