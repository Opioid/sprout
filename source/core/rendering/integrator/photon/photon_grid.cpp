#include "photon_grid.hpp"
#include <algorithm>
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/thread/thread_pool.hpp"
#include "scene/entity/composed_transformation.inl"
#include "scene/material/bxdf.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.inl"
#include "scene/prop/prop.hpp"
#include "scene/prop/prop_intersection.inl"
#include "scene/shape/shape.hpp"

namespace rendering::integrator::photon {

Grid::Grid(float radius, float merge_radius_factor) noexcept
    : num_photons_(0),
      photons_(nullptr),
      photon_radius_(radius),
      inverse_cell_size_(1.f / (Grid_radius_factor * radius)),
      merge_radius_factor_(merge_radius_factor),
      dimensions_(0),
      grid_(nullptr) {}

Grid::~Grid() noexcept {
    memory::free_aligned(grid_);
}

void Grid::resize(math::AABB const& aabb) noexcept {
    aabb_ = aabb;

    int3 const dimensions = map3(aabb.max()) + int3(3);

    int32_t const num_cells = dimensions[0] * dimensions[1] * dimensions[2];

    if (dimensions_ != dimensions) {
        dimensions_ = dimensions;

        memory::free_aligned(grid_);
        grid_ = memory::allocate_aligned<int2>(static_cast<uint32_t>(num_cells));

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
        adjacencies_[0] = {{int2(0), int2(0), int2(0), int2(0)}, 1};

        // 00, 00, 01
        adjacencies_[1] = {{int2(0), int2(o__0__0_p1), int2(0), int2(0)}, 2};

        // 00, 00, 10
        adjacencies_[2] = {{int2(0), int2(o__0__0_m1), int2(0), int2(0)}, 2};
        adjacencies_[3] = {{int2(0), int2(0), int2(0), int2(0)}, 0};

        // 00, 01, 00
        adjacencies_[4] = {{int2(0), int2(o__0_p1__0), int2(0), int2(0)}, 2};

        // 00, 01, 01
        adjacencies_[5] = {{int2(0), int2(o__0_p1__0), int2(o__0__0_p1), int2(o__0_p1_m1)}, 4};

        // 00, 01, 10
        adjacencies_[6] = {{int2(0), int2(o__0_p1__0), int2(o__0__0_m1), int2(o__0_p1_m1)}, 4};
        adjacencies_[7] = {{int2(0), int2(0), int2(0), int2(0)}, 0};

        // 00, 10, 00
        adjacencies_[8] = {{int2(0), int2(o__0_m1__0), int2(0), int2(0)}, 2};

        // 00, 10, 01
        adjacencies_[9] = {{int2(0), int2(o__0_m1__0), int2(o__0__0_p1), int2(o__0_m1_p1)}, 4};

        // 00, 10, 10
        adjacencies_[10] = {{int2(0), int2(o__0_m1__0), int2(o__0__0_m1), int2(o__0_m1_m1)}, 4};
        adjacencies_[11] = {{int2(0), int2(0), int2(0), int2(0)}, 0};
        adjacencies_[12] = {{int2(0), int2(0), int2(0), int2(0)}, 0};
        adjacencies_[13] = {{int2(0), int2(0), int2(0), int2(0)}, 0};
        adjacencies_[14] = {{int2(0), int2(0), int2(0), int2(0)}, 0};
        adjacencies_[15] = {{int2(0), int2(0), int2(0), int2(0)}, 0};

        // 01, 00, 00
        adjacencies_[16] = {{int2(0, o_p1__0__0), int2(0), int2(0), int2(0)}, 1};

        // 01, 00, 01
        adjacencies_[17] = {{int2(0, o_p1__0__0), int2(o__0__0_p1, o_p1__0_p1), int2(0), int2(0)},
                            2};

        // 01, 00, 10
        adjacencies_[18] = {{int2(0, o_p1__0__0), int2(o__0__0_m1, o_p1__0_m1), int2(0), int2(0)},
                            2};
        adjacencies_[19] = {{int2(0), int2(0), int2(0), int2(0)}, 0};

        // 01, 01, 00
        adjacencies_[20] = {{int2(0, o_p1__0__0), int2(o__0_p1__0, o_p1_p1__0), int2(0), int2(0)},
                            2};

        // 01, 01, 01
        adjacencies_[21] = {{int2(0, o_p1__0__0), int2(o__0_p1__0, o_p1_p1__0),
                             int2(o__0__0_p1, o_p1__0_p1), int2(o__0_p1_p1, o_p1_p1_p1)},
                            4};

        // 01, 01, 10
        adjacencies_[22] = {{int2(0, o_p1__0__0), int2(o__0_p1__0, o_p1_p1__0),
                             int2(o__0__0_m1, o_p1__0_m1), int2(o__0_p1_m1, o_p1_p1_m1)},
                            4};
        adjacencies_[23] = {{int2(0), int2(0), int2(0), int2(0)}, 0};

        // 01, 10, 00
        adjacencies_[24] = {{int2(0, o_p1__0__0), int2(o__0_m1__0, o_p1_m1__0), int2(0), int2(0)},
                            2};

        // 01, 10, 01
        adjacencies_[25] = {{int2(0, o_p1__0__0), int2(o__0_m1__0, o_p1_m1__0),
                             int2(o__0__0_p1, o_p1__0_p1), int2(o__0_m1_p1, o_p1_m1_p1)},
                            4};

        // 01, 10, 10
        adjacencies_[26] = {{int2(0, o_p1__0__0), int2(o__0_m1__0, o_p1_m1__0),
                             int2(o__0__0_m1, o_p1__0_m1), int2(o__0_m1_m1, o_p1_m1_m1)},
                            4};
        adjacencies_[27] = {{int2(0), int2(0), int2(0), int2(0)}, 0};
        adjacencies_[28] = {{int2(0), int2(0), int2(0), int2(0)}, 0};
        adjacencies_[29] = {{int2(0), int2(0), int2(0), int2(0)}, 0};
        adjacencies_[30] = {{int2(0), int2(0), int2(0), int2(0)}, 0};
        adjacencies_[31] = {{int2(0), int2(0), int2(0), int2(0)}, 0};

        // 10, 00, 00
        adjacencies_[32] = {{int2(o_m1__0__0, 0), int2(0), int2(0), int2(0)}, 1};

        // 10, 00, 01
        adjacencies_[33] = {{int2(o_m1__0__0, 0), int2(o_m1__0_p1, o__0__0_p1), int2(0), int2(0)},
                            2};

        // 10, 00, 10
        adjacencies_[34] = {{int2(o_m1__0__0, 0), int2(o_m1__0_m1, o__0__0_m1), int2(0), int2(0)},
                            2};
        adjacencies_[35] = {{int2(0), int2(0), int2(0), int2(0)}, 0};

        // 10, 01, 00
        adjacencies_[36] = {{int2(o_m1__0__0, 0), int2(o_m1_p1__0, o__0_p1__0), int2(0), int2(0)},
                            2};

        // 10, 01, 01
        adjacencies_[37] = {{int2(o_m1__0__0, 0), int2(o_m1_p1__0, o__0_p1__0),
                             int2(o_m1__0_p1, o__0__0_p1), int2(o_m1_p1_p1, o__0_p1_p1)},
                            4};

        // 10, 01, 10
        adjacencies_[38] = {{int2(o_m1__0__0, 0), int2(o_m1_p1__0, o__0_p1__0),
                             int2(o_m1__0_m1, o__0__0_m1), int2(o_m1_p1_m1, o__0_p1_m1)},
                            4};
        adjacencies_[39] = {{int2(0), int2(0), int2(0), int2(0)}, 0};

        // 10, 10, 00
        adjacencies_[40] = {{int2(o_m1__0__0, 0), int2(o_m1_m1__0, o__0_m1__0), int2(0), int2(0)},
                            2};

        // 10, 10, 01
        adjacencies_[41] = {{int2(o_m1__0__0, 0), int2(o_m1_m1__0, o__0_m1__0),
                             int2(o_m1__0_p1, o__0__0_p1), int2(o_m1_m1_p1, o__0_m1_p1)},
                            4};

        // 10, 10, 10
        adjacencies_[42] = {{int2(o_m1__0__0, 0), int2(o_m1_m1__0, o__0_m1__0),
                             int2(o_m1__0_m1, o__0__0_m1), int2(o_m1_m1_m1, o__0_m1_m1)},
                            4};
    }
}

void Grid::update(uint32_t num_photons, Photon* photons) noexcept {
    num_photons_ = num_photons;
    photons_     = photons;

    if (0 == num_photons) {
        return;
    }

    std::sort(photons,
              photons + num_photons, [this](Photon const& a, Photon const& b) noexcept->bool {
                  int32_t const ida = map1(a.p);
                  int32_t const idb = map1(b.p);

                  return ida < idb;
              });

    int32_t const num_cells = dimensions_[0] * dimensions_[1] * dimensions_[2];

    int32_t const len = static_cast<int32_t>(num_photons);

    int32_t current = 0;
    for (int32_t c = 0; c < num_cells; ++c) {
        int32_t const begin = current;
        for (; current < len; ++current) {
            if (map1(photons[current].p) != c) {
                break;
            }
        }

        grid_[c][0] = begin;
        grid_[c][1] = current;
    }
}

uint32_t Grid::reduce_and_move(Photon* photons, uint32_t* num_reduced,
                               thread::Pool& pool) noexcept {
    pool.run_range([this, num_reduced](uint32_t id, int32_t begin,
                                       int32_t end) { num_reduced[id] = reduce(begin, end); },
                   0, static_cast<int32_t>(num_photons_));

    uint32_t comp_num_photons = num_photons_;

    for (uint32_t i = 0, len = pool.num_threads(); i < len; ++i) {
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

    update(comp_num_photons, photons_);

    return comp_num_photons;
}

static inline float kernel(float squared_distance, float inv_squared_radius) {
    float const s = 1.f - squared_distance * inv_squared_radius;
    return (3.f * math::Pi_inv) * (s * s);
}

float3 Grid::li(Intersection const& intersection, Material_sample const& sample, uint32_t num_paths,
                scene::Worker const& worker) const noexcept {
    if (0 == num_photons_) {
        return float3::identity();
    }

    float3 const position = intersection.geo.p;

    if (!aabb_.intersect(position)) {
        return float3::identity();
    }

    float3 result = float3::identity();

    Adjacency adjacency;
    adjacent_cells(position, adjacency);

    if (intersection.subsurface) {
        float const radius_2 = photon_radius_ * photon_radius_;
        float const radius_3 = photon_radius_ * radius_2;

        for (uint32_t c = 0; c < adjacency.num_cells; ++c) {
            int2 const cell = adjacency.cells[c];

            for (int32_t i = cell[0], len = cell[1]; i < len; ++i) {
                auto const& photon = photons_[i];

                if (photon.properties.test_not(Photon::Property::Volumetric)) {
                    continue;
                }

                if (math::squared_distance(photon.p, position) <= radius_2) {
                    auto const bxdf = sample.evaluate(photon.wi, true);

                    result += float3(photon.alpha) * bxdf.reflection;
                }
            }
        }

        float3 const mu_s = scattering_coefficient(intersection, worker);

        result /= (((4.f / 3.f) * math::Pi) * (radius_3 * static_cast<float>(num_paths))) * mu_s;
    } else {
        float const radius_2     = photon_radius_ * photon_radius_;
        float const inv_radius_2 = 1.f / radius_2;

        for (uint32_t c = 0; c < adjacency.num_cells; ++c) {
            int2 const cell = adjacency.cells[c];

            for (int32_t i = cell[0], len = cell[1]; i < len; ++i) {
                auto const& photon = photons_[i];

                if (photon.properties.test(Photon::Property::Volumetric)) {
                    continue;
                }

                if (float const distance_2 = math::squared_distance(photon.p, position);
                    distance_2 <= radius_2) {
                    if (float const n_dot_wi = sample.base_layer().abs_n_dot(photon.wi);
                        n_dot_wi > 0.f) {
                        float const clamped_n_dot_wi = scene::material::clamp(n_dot_wi);

                        float const k = kernel(distance_2, inv_radius_2);

                        auto const bxdf = sample.evaluate(photon.wi, true);

                        result += (k / clamped_n_dot_wi) * (float3(photon.alpha) * bxdf.reflection);
                    }
                }
            }
        }

        result /= static_cast<float>(num_paths) * radius_2;
    }

    return result;
}

size_t Grid::num_bytes() const noexcept {
    int32_t const num_cells = dimensions_[0] * dimensions_[1] * dimensions_[2];

    size_t const num_bytes = static_cast<uint32_t>(num_cells) * sizeof(int2);

    return num_bytes;
}

uint32_t Grid::reduce(int32_t begin, int32_t end) noexcept {
    float const merge_distance = math::pow2(merge_radius_factor_ * photon_radius_);

    uint32_t num_reduced = 0;

    for (int32_t i = begin, ilen = end; i < ilen; ++i) {
        auto& pa = photons_[i];

        if (pa.alpha[0] < 0.f) {
            continue;
        }

        float3 position = pa.p;

        uint32_t local_reduced = 0;

        Adjacency adjacency;
        adjacent_cells(pa.p, adjacency);

        for (uint32_t c = 0; c < adjacency.num_cells; ++c) {
            int2 const cell = adjacency.cells[c];

            for (int32_t j = std::max(cell[0], i + 1), jlen = std::min(cell[1], end); j < jlen;
                 ++j) {
                auto& pb = photons_[j];

                if (pb.alpha[0] < 0.f) {
                    continue;
                }

                if (math::squared_distance(pa.p, pb.p) < merge_distance) {
                    position += pb.p;

                    float3 const sum = float3(pa.alpha) + float3(pb.alpha);

                    pa.alpha[0] = sum[0];
                    pa.alpha[1] = sum[1];
                    pa.alpha[2] = sum[2];

                    pb.alpha[0] = -1.f;
                    ++num_reduced;
                }
            }
        }

        if (local_reduced > 0) {
            pa.p = position / static_cast<float>(local_reduced + 1);
        }

        num_reduced += local_reduced;
    }

    return num_reduced;
}

enum Adjacent { None = 0, Positive = 1, Negative = 2 };

uint8_t Grid::adjacent(float s) noexcept {
    if (s <= Lower_cell_bound) {
        return Negative;
    }

    if (s >= Upper_cell_bound) {
        return Positive;
    }

    return None;
}

int32_t Grid::map1(float3 const& v) const noexcept {
    int3 const c = static_cast<int3>(inverse_cell_size_ * (v - aabb_.min())) + int3(1);

    return (c[2] * dimensions_[1] + c[1]) * dimensions_[0] + c[0];
}

int3 Grid::map3(float3 const& v) const noexcept {
    return static_cast<int3>(inverse_cell_size_ * (v - aabb_.min()));
}

int3 Grid::map3(float3 const& v, uint8_t& adjacent) const noexcept {
    float3 const r = inverse_cell_size_ * (v - aabb_.min());

    int3 const c = static_cast<int3>(r);

    float3 const d = r - static_cast<float3>(c);

    adjacent = static_cast<uint8_t>(Grid::adjacent(d[0]) << 4);
    adjacent |= static_cast<uint8_t>(Grid::adjacent(d[1]) << 2);
    adjacent |= Grid::adjacent(d[2]);

    return c + int3(1);
}

void Grid::adjacent_cells(float3 const& v, Adjacency& adjacency) const noexcept {
    uint8_t    adjacent;
    int3 const c = map3(v, adjacent);

    int32_t const ic = (c[2] * dimensions_[1] + c[1]) * dimensions_[0] + c[0];

    adjacency = adjacencies_[adjacent];

    for (uint32_t i = 0; i < adjacency.num_cells; ++i) {
        int2 const cells      = adjacency.cells[i];
        adjacency.cells[i][0] = grid_[cells[0] + ic][0];
        adjacency.cells[i][1] = grid_[cells[1] + ic][1];
    }
}

float3 Grid::scattering_coefficient(Intersection const&  intersection,
                                    scene::Worker const& worker) noexcept {
    using Filter_settings = scene::material::Sampler_settings::Filter;

    auto const& material = *intersection.material();

    if (material.is_heterogeneous_volume()) {
        scene::entity::Composed_transformation temp;
        auto const& transformation = intersection.prop->transformation_at(0, temp);

        float3 const local_position = transformation.world_to_object_point(intersection.geo.p);

        auto const shape = intersection.prop->shape();

        float3 const uvw = shape->object_to_texture_point(local_position);

        return material.collision_coefficients(uvw, Filter_settings::Undefined, worker).s;
    } else if (material.is_textured_volume()) {
        return material
            .collision_coefficients(intersection.geo.uv, Filter_settings::Undefined, worker)
            .s;
    } else {
        return material.collision_coefficients().s;
    }
}

}  // namespace rendering::integrator::photon
