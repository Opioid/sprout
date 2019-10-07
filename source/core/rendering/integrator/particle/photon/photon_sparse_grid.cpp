#include "photon_sparse_grid.hpp"
#include <algorithm>
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
#include "scene/prop/prop.hpp"
#include "scene/prop/prop_intersection.inl"
#include "scene/shape/shape.hpp"

#include <iostream>

namespace rendering::integrator::particle::photon {

using namespace scene;

static inline bool operator<(Photon_ref a, Photon_ref b) noexcept {
    return a.sd < b.sd;
}

enum Adjacent { None = 0, Positive = 1, Negative = 2 };

static inline uint8_t adjacent(float s, float2 cell_bound) noexcept {
    if (s < cell_bound[0]) {
        return Negative;
    }

    if (s > cell_bound[1]) {
        return Positive;
    }

    return None;
}

static float3 scattering_coefficient(prop::Intersection const& intersection,
                                     Worker const&             worker) noexcept;

Sparse_grid::Sparse_grid(float search_radius, float grid_cell_factor, bool check_disk) noexcept
    : num_photons_(0),
      photons_(nullptr),
      search_radius_(search_radius),
      grid_cell_factor_(grid_cell_factor),
      cell_bound_(0.5f / grid_cell_factor, 1.f - (0.5f / grid_cell_factor)),
      dimensions_(0),
      cells_(nullptr),
      check_disk_(check_disk) {}

Sparse_grid::~Sparse_grid() noexcept {
    release();
}

void Sparse_grid::resize(AABB const& aabb) noexcept {
    aabb_ = aabb;

    float const diameter = 2.f * search_radius_;

    int3 const dimensions = int3(ceil(aabb.extent() / (diameter * grid_cell_factor_))) + int(2);

    if (dimensions_ != dimensions) {
        release();

        dimensions_ = dimensions;

        local_to_texture_ = 1.f / aabb_.extent() * float3(dimensions - int3(2));

        num_cells_ = dimensions >> Log2_cell_dim;

        num_cells_ += math::min(dimensions - (num_cells_ << Log2_cell_dim), 1);

        int32_t const cell_len = num_cells_[0] * num_cells_[1] * num_cells_[2];

        cells_ = memory::allocate_aligned<Cell>(cell_len);

        for (int32_t i = 0; i < cell_len; ++i) {
            cells_[i].data  = nullptr;
            cells_[i].value = -1;
        }

        //        int32_t const num_cells = dimensions[0] * dimensions[1] * dimensions[2] + 1;

        //        grid_ = memory::allocate_aligned<int32_t>(uint32_t(num_cells));

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

void Sparse_grid::init_cells(uint32_t num_photons, Photon* photons) noexcept {
    num_photons_ = num_photons;
    photons_     = photons;

    if (0 == num_photons) {
        return;
    }

    std::sort(photons, photons + num_photons, [this](Photon const& a, Photon const& b) noexcept {
        int32_t const ida = map1(a.p);
        int32_t const idb = map1(b.p);

        return ida < idb;
    });

    //   int32_t const num_cells = dimensions_[0] * dimensions_[1] * dimensions_[2] + 1;

    int32_t const len = static_cast<int32_t>(num_photons);

    int32_t current = 0;
    for (int32_t z = 0, zlen = dimensions_[2]; z < zlen; ++z) {
        for (int32_t y = 0, ylen = dimensions_[1]; y < ylen; ++y) {
            for (int32_t x = 0, xlen = dimensions_[0]; x < xlen; ++x) {
                int3 const c(x, y, z);
                set(c, current);

                for (; current < len; ++current) {
                    if (map3(photons[current].p) != c) {
                        break;
                    }
                }
            }
        }
    }

    /*
        int32_t current = 0;
        for (int32_t c = 0; c < num_cells; ++c) {
            grid_[c] = current;
            for (; current < len; ++current) {
                if (map1(photons[current].p) != c) {
                    break;
                }
            }
        }
    */
}

uint32_t Sparse_grid::reduce_and_move(Photon* photons, float merge_radius, uint32_t* num_reduced,
                                      thread::Pool& pool) noexcept {
    pool.run_range(
        [this, merge_radius, num_reduced](uint32_t id, int32_t begin, int32_t end) noexcept {
            num_reduced[id] = reduce(merge_radius, begin, end);
        },
        0, static_cast<int32_t>(num_photons_));

    uint32_t comp_num_photons = num_photons_;

    for (uint32_t i = 0, len = pool.num_threads(); i < len; ++i) {
        comp_num_photons -= num_reduced[i];
    }

    std::partition(photons_, photons_ + num_photons_,
                   [](Photon const& p) noexcept { return p.alpha[0] >= 0.f; });

    if (photons != photons_) {
        Photon* old_photons = photons_;
        photons_            = photons;

        for (uint32_t i = 0; i < comp_num_photons; ++i) {
            photons[i] = old_photons[i];
        }
    }

    return comp_num_photons;
}

static inline float cone_filter(float squared_distance, float inv_squared_radius) noexcept {
    float const s = 1.f - squared_distance * inv_squared_radius;

    return s * s;
}

static inline float conely_filter(float squared_distance, float inv_squared_radius) noexcept {
    float const s = 1.f - squared_distance * inv_squared_radius;

    return s;
}

void Sparse_grid::set_num_paths(uint64_t num_paths) noexcept {
    float const radius_2   = search_radius_ * search_radius_;
    surface_normalization_ = 1.f / (((1.f / 2.f) * Pi) * float(num_paths) * radius_2);

    float const radius_3  = search_radius_ * radius_2;
    volume_normalization_ = 1.f / (((4.f / 3.f) * Pi) * (radius_3 * float(num_paths)));
}

float3 Sparse_grid::li(Intersection const& intersection, Material_sample const& sample,
                       Photon_ref* /*photon_refs*/, scene::Worker const& worker) const noexcept {
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

    if (intersection.subsurface) {
        float const radius_2 = search_radius_ * search_radius_;

        for (uint32_t c = 0; c < adjacency.num_cells; ++c) {
            int2 const cell = adjacency.cells[c];

            for (int32_t i = cell[0], len = cell[1]; i < len; ++i) {
                auto const& photon = photons_[i];

                if (photon.properties.no(Photon::Property::Volumetric)) {
                    continue;
                }

                if (squared_distance(photon.p, position) <= radius_2) {
                    auto const bxdf = sample.evaluate_f(photon.wi, true);

                    result += float3(photon.alpha) * bxdf.reflection;
                }
            }
        }

        float3 const mu_s = scattering_coefficient(intersection, worker);

        result *= volume_normalization_ / mu_s;
    } else {
        float const radius_2 = search_radius_ * search_radius_;

        float const inv_radius_2 = 1.f / radius_2;

        Plane const disk = plane::create(intersection.geo.n, position);

        float const disk_thickness = search_radius_ * 0.125f;

        for (uint32_t c = 0; c < adjacency.num_cells; ++c) {
            int2 const cell = adjacency.cells[c];

            for (int32_t i = cell[0], len = cell[1]; i < len; ++i) {
                auto const& photon = photons_[i];

                if (photon.properties.is(Photon::Property::Volumetric)) {
                    continue;
                }

                if (float const distance_2 = squared_distance(photon.p, position);
                    distance_2 < radius_2) {
                    if (check_disk_ && std::abs(plane::dot(disk, photon.p)) > disk_thickness) {
                        continue;
                    }

                    if (sample.base_layer().n_dot(photon.wi) > 0.f) {
                        // float const k = 1.f;

                        // float const k = cone_filter(distance_2, inv_radius_2);

                        float const k = conely_filter(distance_2, inv_radius_2);

                        auto const bxdf = sample.evaluate_b(photon.wi, true);

                        result += k * float3(photon.alpha) * bxdf.reflection;
                    }
                }
            }
        }

        // cone_filter
        //  result /= ((1.f / 3.f) * Pi) * float(num_paths) * radius_2;

        // conely_filter
        result *= surface_normalization_;

        // unfiltered
        //   result /= Pi * float(num_paths) * radius_2;
    }

    return result;
}

size_t Sparse_grid::num_bytes() const noexcept {
    int32_t const num_cells = dimensions_[0] * dimensions_[1] * dimensions_[2] + 1;

    size_t const num_bytes = size_t(num_cells) * sizeof(int32_t);

    return num_bytes;
}

void Sparse_grid::release() noexcept {
    int32_t const cell_len = num_cells_[0] * num_cells_[1] * num_cells_[2];

    for (int32_t i = 0; i < cell_len; ++i) {
        memory::free_aligned(cells_[i].data);
    }

    memory::free_aligned(cells_);
}

void Sparse_grid::set(int3 const& c, int32_t value) noexcept {
    int3 const cc = c >> Log2_cell_dim;

    int32_t const cell_index = (cc[2] * num_cells_[1] + cc[1]) * num_cells_[0] + cc[0];

    Cell& cell = cells_[cell_index];

    if (cell.data) {
        int3 const cs = cc << Log2_cell_dim;

        int3 const cxyz = c - cs;

        int32_t const ci = (((cxyz[2] << Log2_cell_dim) + cxyz[1]) << Log2_cell_dim) + cxyz[0];

        cell.data[ci] = value;
    } else if (cell.value >= 0 && cell.value != value) {
        cell.data = memory::allocate_aligned<int32_t>(Cell_dim * Cell_dim * Cell_dim);

        cell.data[0] = cell.value;
        cell.data[1] = value;
    } else {
        cell.value = value;
    }
}

uint32_t Sparse_grid::reduce(float merge_radius, int32_t begin, int32_t end) noexcept {
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

int32_t Sparse_grid::map1(float3 const& v) const noexcept {
    int3 const c = static_cast<int3>((v - aabb_.min()) * local_to_texture_) + 1;

    return (c[2] * dimensions_[1] + c[1]) * dimensions_[0] + c[0];
}

int3 Sparse_grid::map3(float3 const& v) const noexcept {
    float3 const r = (v - aabb_.min()) * local_to_texture_;

    int3 const c = static_cast<int3>(r);

    return c + 1;
}

int3 Sparse_grid::map3(float3 const& v, float2 cell_bound, uint8_t& adjacents) const noexcept {
    float3 const r = (v - aabb_.min()) * local_to_texture_;

    int3 const c = static_cast<int3>(r);

    float3 const d = r - static_cast<float3>(c);

    adjacents = uint8_t(adjacent(d[0], cell_bound) << 4);
    adjacents |= uint8_t(adjacent(d[1], cell_bound) << 2);
    adjacents |= adjacent(d[2], cell_bound);

    return c + 1;
}

void Sparse_grid::adjacent_cells(float3 const& v, float2 cell_bound, Adjacency& adjacency) const
    noexcept {
    uint8_t    adjacents;
    int3 const c = map3(v, cell_bound, adjacents);

    int32_t const ic = (c[2] * dimensions_[1] + c[1]) * dimensions_[0] + c[0];

    adjacency = adjacencies_[adjacents];

    for (uint32_t i = 0; i < adjacency.num_cells; ++i) {
        int2 const cells = adjacency.cells[i];

        adjacency.cells[i][0] = 0;
        adjacency.cells[i][1] = 0;

        //    adjacency.cells[i][0] = grid_[cells[0] + ic];
        //    adjacency.cells[i][1] = grid_[cells[1] + ic + 1];
    }
}

static float3 scattering_coefficient(prop::Intersection const& intersection,
                                     Worker const&             worker) noexcept {
    using Filter = material::Sampler_settings::Filter;

    auto const& material = *intersection.material(worker);

    if (material.is_heterogeneous_volume()) {
        entity::Composed_transformation temp;
        auto const& transformation = worker.scene().prop_transformation_at(intersection.prop, 0,
                                                                           temp);

        float3 const local_position = transformation.world_to_object_point(intersection.geo.p);

        auto const shape = worker.scene().prop_shape(intersection.prop);

        float3 const uvw = shape->object_to_texture_point(local_position);

        return material.collision_coefficients(uvw, Filter::Undefined, worker).s;
    } else if (material.is_textured_volume()) {
        return material.collision_coefficients(intersection.geo.uv, Filter::Undefined, worker).s;
    } else {
        return material.collision_coefficients().s;
    }
}

}  // namespace rendering::integrator::particle::photon
