#include "photon_map.hpp"
#include "base/math/aabb.inl"
#include "base/memory/align.hpp"
#include "base/memory/array.inl"
#include "base/thread/thread_pool.hpp"
#include "photon.hpp"
#include "scene/scene_worker.hpp"

#include <string>

#include <iostream>
#include "base/math/print.hpp"

namespace rendering::integrator::particle::photon {

Map::Map() : aabbs_(nullptr), num_reduced_(nullptr) {}

Map::~Map() {
    memory::free_aligned(num_reduced_);
    memory::free_aligned(aabbs_);
}

void Map::init(uint32_t num_workers, uint32_t num_photons, float search_radius,
               float merge_radius) {
    merge_radius_ = merge_radius;

    grid_.init(search_radius, 1.5f, false);

    photons_.resize(num_photons);

    if (!aabbs_) {
        aabbs_       = memory::allocate_aligned<AABB>(num_workers);
        num_reduced_ = memory::allocate_aligned<uint32_t>(num_workers);
    }
}

void Map::start() {
    reduced_num_  = 0;
    caustic_only_ = false;
}

void Map::insert(Photon const& photon, uint32_t index) {
    photons_[index] = photon;
}

uint32_t Map::compile_iteration(uint32_t num_photons, uint64_t num_paths, Threads& threads) {
    AABB const aabb = calculate_aabb(num_photons, threads);

    grid_.resize(aabb);

    num_paths_ = num_paths;

    grid_.init_cells(num_photons, photons_.data());

    uint32_t const total_num_photons = photons_.size();

    uint32_t const reduced_num = num_photons == total_num_photons
                                     ? grid_.reduce_and_move(photons_.data(), merge_radius_,
                                                             num_reduced_, threads)
                                     : num_photons;

    float const percentage_caustics = float(reduced_num) / float(total_num_photons);

    std::cout << reduced_num << " total left of " << total_num_photons << " ("
              << uint32_t(100.f * percentage_caustics) << "%)" << std::endl;

    reduced_num_ = reduced_num;

    return reduced_num;
}

void Map::compile_finalize() {
    grid_.init_cells(reduced_num_, photons_.data());
    grid_.set_num_paths(num_paths_);
}

float3 Map::li(Intersection const& intersection, Material_sample const& sample,
               scene::Worker const& worker) const {
    return grid_.li(intersection, sample, worker);
}

bool Map::caustics_only() const {
    return caustic_only_;
}

AABB Map::calculate_aabb(uint32_t num_photons, Threads& threads) const {
    threads.run_range(
        [this](uint32_t id, int32_t begin, int32_t end) {
            AABB aabb = AABB::empty();

            for (int32_t i = begin; i < end; ++i) {
                aabb.insert(photons_[i].p);
            }

            aabbs_[id] = aabb;
        },
        0, int32_t(num_photons));

    AABB aabb = AABB::empty();

    for (uint32_t i = 0, len = threads.num_threads(); i < len; ++i) {
        aabb.merge_assign(aabbs_[i]);
    }

    aabb.add(0.0001f);

    return aabb;
}

}  // namespace rendering::integrator::particle::photon
