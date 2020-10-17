#include "photon_grid.hpp"
#include "base/math/aabb.inl"
#include "base/math/plane.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/thread/thread_pool.hpp"
#include "photon.hpp"
#include "scene/entity/composed_transformation.inl"
#include "scene/material/bxdf.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene_worker.inl"
#include "scene/shape/shape.hpp"

#include <algorithm>

#include <iostream>
#include "base/math/print.hpp"

namespace rendering::integrator::particle::photon {

using namespace scene;

enum Adjacent { None = 0, Positive = 1, Negative = 2 };

static inline uint8_t adjacent(float s, float2 cell_bound) {
    if (s < cell_bound[0]) {
        return Negative;
    }

    if (s > cell_bound[1]) {
        return Positive;
    }

    return None;
}

static float3 scattering_coefficient(prop::Intersection const& intersection, Worker const& worker);

Grid::Grid() : dimensions_(0), grid_(nullptr) {}

Grid::~Grid() {
    memory::free_aligned(grid_);
}

void Grid::init(float search_radius, float grid_cell_factor, bool check_disk) {
    search_radius_ = search_radius;

    grid_cell_factor_ = grid_cell_factor;

    cell_bound_ = float2(0.5f / grid_cell_factor, 1.f - (0.5f / grid_cell_factor));

    check_disk_ = check_disk;
}

void Grid::resize(AABB const& aabb) {
    aabb_ = aabb;

    float const diameter = 2.f * search_radius_;

    int3 const dimensions = int3(ceil(aabb.extent() / (diameter * grid_cell_factor_))) + int(2);

    if (dimensions_ != dimensions) {
        std::cout << dimensions << std::endl;

        dimensions_ = dimensions;

        local_to_texture_ = 1.f / aabb.extent() * float3(dimensions - int3(2));

        int32_t const num_cells = dimensions[0] * dimensions[1] * dimensions[2] + 1;

        memory::free_aligned(grid_);
        grid_ = memory::allocate_aligned<int32_t>(uint32_t(num_cells));

        int32_t const area = dimensions[0] * dimensions[1];

        static int32_t constexpr o_m1__0__0 = -1;
        static int32_t constexpr o_p1__0__0 = +1;

        int32_t const o__0_m1__0 = -dimensions[0];
        int32_t const o__0_p1__0 = +dimensions[0];

        int32_t const o__0__0_m1 = -area;
        int32_t const o__0__0_p1 = +area;

        int32_t const o_m1_m1__0 = -1 - dimensions[0];
        int32_t const o_m1_p1__0 = -1 + dimensions[0];
        int32_t const o_p1_m1__0 = +1 - dimensions[0];
        int32_t const o_p1_p1__0 = +1 + dimensions[0];

        int32_t const o_m1_m1_m1 = -1 - dimensions[0] - area;
        int32_t const o_m1_m1_p1 = -1 - dimensions[0] + area;
        int32_t const o_m1_p1_m1 = -1 + dimensions[0] - area;
        int32_t const o_m1_p1_p1 = -1 + dimensions[0] + area;

        int32_t const o_p1_m1_m1 = +1 - dimensions[0] - area;
        int32_t const o_p1_m1_p1 = +1 - dimensions[0] + area;
        int32_t const o_p1_p1_m1 = +1 + dimensions[0] - area;
        int32_t const o_p1_p1_p1 = +1 + dimensions[0] + area;

        int32_t const o_m1__0_m1 = -1 - area;
        int32_t const o_m1__0_p1 = -1 + area;
        int32_t const o_p1__0_m1 = +1 - area;
        int32_t const o_p1__0_p1 = +1 + area;

        int32_t const o__0_m1_m1 = -dimensions[0] - area;
        int32_t const o__0_m1_p1 = -dimensions[0] + area;
        int32_t const o__0_p1_m1 = +dimensions[0] - area;
        int32_t const o__0_p1_p1 = +dimensions[0] + area;

        // 00, 00, 00
        adjacencies_[0] = {1, {int2(0), int2(0), int2(0), int2(0)}};

        // 00, 00, 01
        adjacencies_[1] = {2, {int2(0), int2(o__0__0_p1), int2(0), int2(0)}};

        // 00, 00, 10
        adjacencies_[2] = {2, {int2(o__0__0_m1), int2(0), int2(0), int2(0)}};
        adjacencies_[3] = {0, {int2(0), int2(0), int2(0), int2(0)}};

        // 00, 01, 00
        adjacencies_[4] = {2, {int2(0), int2(o__0_p1__0), int2(0), int2(0)}};

        // 00, 01, 01
        adjacencies_[5] = {4, {int2(0), int2(o__0_p1__0), int2(o__0__0_p1), int2(o__0_p1_p1)}};

        // 00, 01, 10
        adjacencies_[6] = {4, {int2(o__0__0_m1), int2(o__0_p1_m1), int2(0), int2(o__0_p1__0)}};
        adjacencies_[7] = {0, {int2(0), int2(0), int2(0), int2(0)}};

        // 00, 10, 00
        adjacencies_[8] = {2, {int2(o__0_m1__0), int2(0), int2(0), int2(0)}};

        // 00, 10, 01
        adjacencies_[9] = {4, {int2(o__0_m1__0), int2(o__0_m1_p1), int2(0), int2(o__0__0_p1)}};

        // 00, 10, 10
        adjacencies_[10] = {4, {int2(o__0_m1_m1), int2(o__0__0_m1), int2(o__0_m1__0), int2(0)}};
        adjacencies_[11] = {0, {int2(0), int2(0), int2(0), int2(0)}};
        adjacencies_[12] = {0, {int2(0), int2(0), int2(0), int2(0)}};
        adjacencies_[13] = {0, {int2(0), int2(0), int2(0), int2(0)}};
        adjacencies_[14] = {0, {int2(0), int2(0), int2(0), int2(0)}};
        adjacencies_[15] = {0, {int2(0), int2(0), int2(0), int2(0)}};

        // 01, 00, 00
        adjacencies_[16] = {1, {int2(0, o_p1__0__0), int2(0), int2(0), int2(0)}};

        // 01, 00, 01
        adjacencies_[17] = {2,
                            {int2(0, o_p1__0__0), int2(o__0__0_p1, o_p1__0_p1), int2(0), int2(0)}};

        // 01, 00, 10
        adjacencies_[18] = {2,
                            {int2(o__0__0_m1, o_p1__0_m1), int2(0, o_p1__0__0), int2(0), int2(0)}};
        adjacencies_[19] = {0, {int2(0), int2(0), int2(0), int2(0)}};

        // 01, 01, 00
        adjacencies_[20] = {2,
                            {int2(0, o_p1__0__0), int2(o__0_p1__0, o_p1_p1__0), int2(0), int2(0)}};

        // 01, 01, 01
        adjacencies_[21] = {4,
                            {int2(0, o_p1__0__0), int2(o__0_p1__0, o_p1_p1__0),
                             int2(o__0__0_p1, o_p1__0_p1), int2(o__0_p1_p1, o_p1_p1_p1)}};

        // 01, 01, 10
        adjacencies_[22] = {4,
                            {int2(o__0__0_m1, o_p1__0_m1), int2(o__0_p1_m1, o_p1_p1_m1),
                             int2(0, o_p1__0__0), int2(o__0_p1__0, o_p1_p1__0)}};
        adjacencies_[23] = {0, {int2(0), int2(0), int2(0), int2(0)}};

        // 01, 10, 00
        adjacencies_[24] = {2,
                            {int2(o__0_m1__0, o_p1_m1__0), int2(0, o_p1__0__0), int2(0), int2(0)}};

        // 01, 10, 01
        adjacencies_[25] = {4,
                            {int2(o__0_m1__0, o_p1_m1__0), int2(o__0_m1_p1, o_p1_m1_p1),
                             int2(0, o_p1__0__0), int2(o__0__0_p1, o_p1__0_p1)}};

        // 01, 10, 10
        adjacencies_[26] = {4,
                            {int2(o__0_m1_m1, o_p1_m1_m1), int2(o__0__0_m1, o_p1__0_m1),
                             int2(o__0_m1__0, o_p1_m1__0), int2(0, o_p1__0__0)}};
        adjacencies_[27] = {0, {int2(0), int2(0), int2(0), int2(0)}};
        adjacencies_[28] = {0, {int2(0), int2(0), int2(0), int2(0)}};
        adjacencies_[29] = {0, {int2(0), int2(0), int2(0), int2(0)}};
        adjacencies_[30] = {0, {int2(0), int2(0), int2(0), int2(0)}};
        adjacencies_[31] = {0, {int2(0), int2(0), int2(0), int2(0)}};

        // 10, 00, 00
        adjacencies_[32] = {1, {int2(o_m1__0__0, 0), int2(0), int2(0), int2(0)}};

        // 10, 00, 01
        adjacencies_[33] = {2,
                            {int2(o_m1__0__0, 0), int2(o_m1__0_p1, o__0__0_p1), int2(0), int2(0)}};

        // 10, 00, 10
        adjacencies_[34] = {2,
                            {int2(o_m1__0_m1, o__0__0_m1), int2(o_m1__0__0, 0), int2(0), int2(0)}};
        adjacencies_[35] = {0, {int2(0), int2(0), int2(0), int2(0)}};

        // 10, 01, 00
        adjacencies_[36] = {2,
                            {int2(o_m1__0__0, 0), int2(o_m1_p1__0, o__0_p1__0), int2(0), int2(0)}};

        // 10, 01, 01
        adjacencies_[37] = {4,
                            {int2(o_m1__0__0, 0), int2(o_m1__0_p1, o__0__0_p1),
                             int2(o_m1_p1__0, o__0_p1__0), int2(o_m1_p1_p1, o__0_p1_p1)}};

        // 10, 01, 10
        adjacencies_[38] = {4,
                            {int2(o_m1__0_m1, o__0__0_m1), int2(o_m1_p1_m1, o__0_p1_m1),
                             int2(o_m1__0__0, 0), int2(o_m1_p1__0, o__0_p1__0)}};
        adjacencies_[39] = {0, {int2(0), int2(0), int2(0), int2(0)}};

        // 10, 10, 00
        adjacencies_[40] = {2,
                            {int2(o_m1_m1__0, o__0_m1__0), int2(o_m1__0__0, 0), int2(0), int2(0)}};

        // 10, 10, 01
        adjacencies_[41] = {4,
                            {int2(o_m1_m1__0, o__0_m1__0), int2(o_m1_m1_p1, o__0_m1_p1),
                             int2(o_m1__0__0, 0), int2(o_m1__0_p1, o__0__0_p1)}};

        // 10, 10, 10
        adjacencies_[42] = {4,
                            {int2(o_m1_m1_m1, o__0_m1_m1), int2(o_m1__0_m1, o__0__0_m1),
                             int2(o_m1_m1__0, o__0_m1__0), int2(o_m1__0__0, 0)}};
    }
}

void Grid::init_cells(uint32_t num_photons, Photon* photons) {
    num_photons_ = num_photons;
    photons_     = photons;

    if (0 == num_photons) {
        return;
    }

    std::sort(photons, photons + num_photons, [this](Photon const& a, Photon const& b) {
        int32_t const ida = map1(a.p);
        int32_t const idb = map1(b.p);

        return ida < idb;
    });

    int32_t const num_cells = dimensions_[0] * dimensions_[1] * dimensions_[2] + 1;

    int32_t const len = int32_t(num_photons);

    int32_t current = 0;
    for (int32_t c = 0; c < num_cells; ++c) {
        grid_[c] = current;
        for (; current < len; ++current) {
            if (map1(photons[current].p) != c) {
                break;
            }
        }
    }
}

uint32_t Grid::reduce_and_move(Photon* photons, float merge_radius, uint32_t* num_reduced,
                               Threads& threads) {
    threads.run_range(
        [this, merge_radius, num_reduced](uint32_t id, int32_t begin, int32_t end) noexcept {
            num_reduced[id] = reduce(merge_radius, begin, end);
        },
        0, int32_t(num_photons_));

    uint32_t comp_num_photons = num_photons_;

    for (uint32_t i = 0, len = threads.num_threads(); i < len; ++i) {
        comp_num_photons -= num_reduced[i];
    }

    std::partition(photons_, photons_ + num_photons_,
                   [](Photon const& p) { return p.alpha[0] >= 0.f; });

    if (photons != photons_) {
        Photon* old_photons = photons_;
        photons_            = photons;

        for (uint32_t i = 0; i < comp_num_photons; ++i) {
            photons[i] = old_photons[i];
        }
    }

    return comp_num_photons;
}

static inline float cone_filter(float squared_distance, float inv_squared_radius) {
    float const s = 1.f - squared_distance * inv_squared_radius;

    return s * s;
}

static inline float conely_filter(float squared_distance, float inv_squared_radius) {
    float const s = 1.f - squared_distance * inv_squared_radius;

    return s;
}

void Grid::set_num_paths(uint64_t num_paths) {
    float const radius2 = search_radius_ * search_radius_;

    // conely
    surface_normalization_ = 1.f / (((1.f / 2.f) * Pi) * float(num_paths) * radius2);

    // cone
    // surface_normalization_ = 1.f / (((1.f / 3.f) * Pi) * float(num_paths) * radius2);

    float const radius3   = search_radius_ * radius2;
    volume_normalization_ = 1.f / (((4.f / 3.f) * Pi) * (radius3 * float(num_paths)));
}

float3 Grid::li(Intersection const& intersection, Material_sample const& sample,
                scene::Worker const& worker) const {
    if (0 == num_photons_) {
        return float3(0.f);
    }

    float3 const position = intersection.geo.p;

    if (!aabb_.intersect(position)) {
        return float3(0.f);
    }

    float3 result = float3(0.f);

    Adjacency adjacency;
    adjacent_cells(position, cell_bound_, adjacency);

    float const radius2 = search_radius_ * search_radius_;

    if (intersection.subsurface) {
        for (uint32_t c = 0; c < adjacency.num_cells; ++c) {
            int2 const cell = adjacency.cells[c];

            for (int32_t i = cell[0], len = cell[1]; i < len; ++i) {
                auto const& photon = photons_[i];

                if (photon.properties.no(Photon::Property::Volumetric)) {
                    continue;
                }

                if (squared_distance(photon.p, position) <= radius2) {
                    auto const bxdf = sample.evaluate_f(photon.wi);

                    result += float3(photon.alpha) * bxdf.reflection;
                }
            }
        }

        float3 const mu_s = scattering_coefficient(intersection, worker);

        result *= volume_normalization_ / mu_s;
    } else {
        float const inv_radius2 = 1.f / radius2;

        Plane const disk = plane::create(intersection.geo.n, position);

        float const disk_thickness = search_radius_ * 0.125f;

        for (uint32_t c = 0; c < adjacency.num_cells; ++c) {
            int2 const cell = adjacency.cells[c];

            for (int32_t i = cell[0], len = cell[1]; i < len; ++i) {
                auto const& photon = photons_[i];

                if (photon.properties.is(Photon::Property::Volumetric)) {
                    continue;
                }

                if (float const distance2 = squared_distance(photon.p, position);
                    distance2 < radius2) {
                    if (check_disk_ && std::abs(plane::dot(disk, photon.p)) > disk_thickness) {
                        continue;
                    }

                    if (dot(sample.interpolated_normal(), photon.wi) > 0.f) {
                        // float const k = 1.f;

                        // float const k = cone_filter(distance2, inv_radius2);

                        float const k = conely_filter(distance2, inv_radius2);

                        auto const bxdf = sample.evaluate_b(photon.wi);

                        result += k * float3(photon.alpha) * bxdf.reflection;
                    }
                }
            }
        }

        result *= surface_normalization_;

        // unfiltered
        //   result /= Pi * float(num_paths) * radius_2;
    }

    return result;
}

uint32_t Grid::reduce(float merge_radius, int32_t begin, int32_t end) {
    float const merge_grid_cell_factor = (search_radius_ * grid_cell_factor_) / merge_radius;

    float2 const cell_bound(0.5f / merge_grid_cell_factor, 1.f - (0.5f / merge_grid_cell_factor));

    float const merge_radius2 = merge_radius * merge_radius;

    uint32_t num_reduced = 0;

    for (int32_t i = begin, ilen = end; i < ilen; ++i) {
        auto& a = photons_[i];

        if (a.alpha[0] < 0.f) {
            continue;
        }

        float3 a_alpha = float3(a.alpha);

        float total_weight = average(a_alpha);

        float3 position = total_weight * a.p;

        float3 wi = a.wi;

        uint32_t local_reduced = 0;

        Adjacency adjacency;
        adjacent_cells(a.p, cell_bound, adjacency);

        for (uint32_t c = 0; c < adjacency.num_cells; ++c) {
            int2 const cell = adjacency.cells[c];

            for (int32_t j = std::max(cell[0], i + 1), jlen = std::min(cell[1], end); j < jlen;
                 ++j) {
                if (j == i) {
                    continue;
                }

                auto& b = photons_[j];

                if (b.alpha[0] < 0.f) {
                    continue;
                }

                if (squared_distance(a.p, b.p) > merge_radius2) {
                    continue;
                }

                float3 const b_alpha = float3(b.alpha);

                float const weight = average(b_alpha);

                float const ratio = total_weight > weight ? weight / total_weight
                                                          : total_weight / weight;

                float const threshold = std::max(ratio - 0.1f, 0.f);

                if (dot(wi, b.wi) < threshold) {
                    continue;
                }

                a_alpha += b_alpha;

                b.alpha[0] = -1.f;

                if (weight > total_weight) {
                    wi = b.wi;
                }

                total_weight += weight;

                position += weight * b.p;

                ++local_reduced;
            }
        }

        if (local_reduced > 0) {
            if (total_weight < 1.e-10f) {
                a.alpha[0] = -1.f;
                ++local_reduced;
            } else {
                a.p = position / total_weight;

                a.wi = wi;

                a.alpha[0] = a_alpha[0];
                a.alpha[1] = a_alpha[1];
                a.alpha[2] = a_alpha[2];
            }
        }

        num_reduced += local_reduced;
    }

    return num_reduced;
}

int32_t Grid::map1(float3 const& v) const {
    int3 const c = int3((v - aabb_.min()) * local_to_texture_) + 1;

    return (c[2] * dimensions_[1] + c[1]) * dimensions_[0] + c[0];
}

int3 Grid::map3(float3 const& v, float2 cell_bound, uint8_t& adjacents) const {
    float3 const r = (v - aabb_.min()) * local_to_texture_;

    int3 const c = int3(r);

    float3 const d = r - float3(c);

    adjacents = uint8_t(adjacent(d[0], cell_bound) << 4);
    adjacents |= uint8_t(adjacent(d[1], cell_bound) << 2);
    adjacents |= adjacent(d[2], cell_bound);

    return c + 1;
}

void Grid::adjacent_cells(float3 const& v, float2 cell_bound, Adjacency& adjacency) const {
    uint8_t    adjacents;
    int3 const c = map3(v, cell_bound, adjacents);

    int32_t const ic = (c[2] * dimensions_[1] + c[1]) * dimensions_[0] + c[0];

    adjacency = adjacencies_[adjacents];

    for (uint32_t i = 0; i < adjacency.num_cells; ++i) {
        int2 const cells = adjacency.cells[i];

        adjacency.cells[i][0] = grid_[cells[0] + ic];
        adjacency.cells[i][1] = grid_[cells[1] + ic + 1];
    }
}

static float3 scattering_coefficient(prop::Intersection const& intersection, Worker const& worker) {
    using Filter = material::Sampler_settings::Filter;

    auto const& material = *intersection.material(worker);

    if (material.is_heterogeneous_volume()) {
        Scene const& scene = worker.scene();

        entity::Composed_transformation temp;
        auto const& transformation = scene.prop_transformation_at(intersection.prop, 0, temp);

        float3 const local_position = transformation.world_to_object_point(intersection.geo.p);

        auto const shape = scene.prop_shape(intersection.prop);

        float3 const uvw = shape->object_to_texture_point(local_position);

        return material.collision_coefficients(uvw, Filter::Undefined, worker).s;
    }

    if (material.is_textured_volume()) {
        return material.collision_coefficients(intersection.geo.uv, Filter::Undefined, worker).s;
    }

    return material.collision_coefficients().s;
}

}  // namespace rendering::integrator::particle::photon
