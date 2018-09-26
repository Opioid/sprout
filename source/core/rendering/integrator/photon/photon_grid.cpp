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
      inverse_cell_size_(1.f / (2.f * radius)),
      merge_radius_factor_(merge_radius_factor),
      dimensions_(0),
      grid_(nullptr) {}

Grid::~Grid() noexcept {
    memory::free_aligned(grid_);
}

void Grid::resize(math::AABB const& aabb) noexcept {
    aabb_ = aabb;

    int3 const dimensions = map3(aabb.max()) + int3(1);

    int32_t const num_cells = dimensions[0] * dimensions[1] * dimensions[2];

    if (dimensions_ != dimensions) {
        dimensions_ = dimensions;
        max_coords_ = math::max(dimensions - int3(1), 1);

        memory::free_aligned(grid_);
        grid_ = memory::allocate_aligned<int2>(static_cast<uint32_t>(num_cells));

        int32_t const area = dimensions[0] * dimensions[1];

        o__0_m1__0_ = -dimensions[0];
        o__0_p1__0_ = +dimensions[0];

        o__0__0_m1_ = -area;
        o__0__0_p1_ = +area;

        o_m1_m1__0_ = -1 - dimensions[0];
        o_m1_p1__0_ = -1 + dimensions[0];
        o_p1_m1__0_ = +1 - dimensions[0];
        o_p1_p1__0_ = +1 + dimensions[0];

        o_m1_m1_m1_ = -1 - dimensions[0] - area;
        o_m1_m1_p1_ = -1 - dimensions[0] + area;
        o_m1_p1_m1_ = -1 + dimensions[0] - area;
        o_m1_p1_p1_ = -1 + dimensions[0] + area;

        o_p1_m1_m1_ = +1 - dimensions[0] - area;
        o_p1_m1_p1_ = +1 - dimensions[0] + area;
        o_p1_p1_m1_ = +1 + dimensions[0] - area;
        o_p1_p1_p1_ = +1 + dimensions[0] + area;

        o_m1__0_m1_ = -1 - area;
        o_m1__0_p1_ = -1 + area;
        o_p1__0_m1_ = +1 - area;
        o_p1__0_p1_ = +1 + area;

        o__0_m1_m1_ = -dimensions[0] - area;
        o__0_m1_p1_ = -dimensions[0] + area;
        o__0_p1_m1_ = +dimensions[0] - area;
        o__0_p1_p1_ = +dimensions[0] + area;
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

    int2 cells[4];
    adjacent_cells(position, cells);

    if (intersection.subsurface) {
        float const radius_2 = photon_radius_ * photon_radius_;
        float const radius_3 = photon_radius_ * radius_2;

        for (uint32_t c = 0; c < 4; ++c) {
            int2 const cell = cells[c];

            for (int32_t i = cell[0], len = cell[1]; i < len; ++i) {
                auto const& photon = photons_[i];

                if (photon.properties.test_not(Photon::Property::Volumetric)) {
                    continue;
                }

                if (math::squared_distance(photon.p, position) <= radius_2) {
                    auto const bxdf = sample.evaluate(photon.wi);

                    result += float3(photon.alpha) * bxdf.reflection;
                }
            }
        }

        float3 const mu_s = scattering_coefficient(intersection, worker);

        result /= (((4.f / 3.f) * math::Pi) * (radius_3 * static_cast<float>(num_paths))) * mu_s;

    } else {
        float const radius_2     = photon_radius_ * photon_radius_;
        float const inv_radius_2 = 1.f / radius_2;

        for (uint32_t c = 0; c < 4; ++c) {
            int2 const cell = cells[c];

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

                        auto const bxdf = sample.evaluate(photon.wi);

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

        int2 cells[4];
        adjacent_cells(pa.p, cells);

        for (uint32_t c = 0; c < 4; ++c) {
            int2 const cell = cells[c];

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

int32_t Grid::map1(float3 const& v) const noexcept {
    int3 const c = static_cast<int3>(inverse_cell_size_ * (v - aabb_.min()));

    return (c[2] * dimensions_[1] + c[1]) * dimensions_[0] + c[0];
}

int3 Grid::map3(float3 const& v) const noexcept {
    return static_cast<int3>(inverse_cell_size_ * (v - aabb_.min()));
}

static inline int8_t adjacent(float s) {
    if (s < 0.5f) {
        return -1;
    }

    if (s > 0.5f) {
        return 1;
    }

    return 0;
}

int3 Grid::map3(float3 const& v, int8_t adjacent[3]) const noexcept {
    float3 const r = inverse_cell_size_ * (v - aabb_.min());

    int3 const c = static_cast<int3>(r);

    float3 const d = r - static_cast<float3>(c);

    adjacent[0] = photon::adjacent(d[0]);
    adjacent[1] = photon::adjacent(d[1]);
    adjacent[2] = photon::adjacent(d[2]);

    if (math::any_less(c, 0)) {
        return int3(0);
    }

    return c;
}

void Grid::adjacent_cells(float3 const& v, int2 cells[4]) const noexcept {
    int8_t     adjacent[3];
    int3 const c = math::min(map3(v, adjacent), max_coords_);

    int32_t const ic = (c[2] * dimensions_[1] + c[1]) * dimensions_[0] + c[0];

    cells[0] = grid_[ic];
    cells[1] = int2(0);
    cells[2] = int2(0);
    cells[3] = int2(0);

    if (-1 == adjacent[0] && c[0] > 0) {
        cells[0][0] = grid_[ic + o_m1__0__0_][0];

        if (-1 == adjacent[1] && c[1] > 0) {
            cells[1][0] = grid_[ic + o_m1_m1__0_][0];
            cells[1][1] = grid_[ic + o__0_m1__0_][1];

            if (-1 == adjacent[2] && c[2] > 0) {
                cells[3][0] = grid_[ic + o_m1_m1_m1_][0];
                cells[3][1] = grid_[ic + o__0_m1_m1_][1];
            } else if (1 == adjacent[2] && c[2] < dimensions_[2] - 1) {
                cells[3][0] = grid_[ic + o_m1_m1_p1_][0];
                cells[3][1] = grid_[ic + o__0_m1_p1_][1];
            }
        } else if (1 == adjacent[1] && c[1] < dimensions_[1] - 1) {
            cells[1][0] = grid_[ic + o_m1_p1__0_][0];
            cells[1][1] = grid_[ic + o__0_p1__0_][1];

            if (-1 == adjacent[2] && c[2] > 0) {
                cells[3][0] = grid_[ic + o_m1_p1_m1_][0];
                cells[3][1] = grid_[ic + o__0_p1_m1_][1];
            } else if (1 == adjacent[2] && c[2] < dimensions_[2] - 1) {
                cells[3][0] = grid_[ic + o_m1_p1_p1_][0];
                cells[3][1] = grid_[ic + o__0_p1_p1_][1];
            }
        }

        if (-1 == adjacent[2] && c[2] > 0) {
            cells[2][0] = grid_[ic + o_m1__0_m1_][0];
            cells[2][1] = grid_[ic + o__0__0_m1_][1];
        } else if (1 == adjacent[2] && c[2] < dimensions_[2] - 1) {
            cells[2][0] = grid_[ic + o_m1__0_p1_][0];
            cells[2][1] = grid_[ic + o__0__0_p1_][1];
        }

        return;
    } else if (1 == adjacent[0] && c[0] < dimensions_[0] - 1) {
        cells[0][1] = grid_[ic + o_p1__0__0_][1];

        if (-1 == adjacent[1] && c[1] > 0) {
            cells[1][0] = grid_[ic + o__0_m1__0_][0];
            cells[1][1] = grid_[ic + o_p1_m1__0_][1];

            if (-1 == adjacent[2] && c[2] > 0) {
                cells[3][0] = grid_[ic + o__0_m1_m1_][0];
                cells[3][1] = grid_[ic + o_p1_m1_m1_][1];
            } else if (1 == adjacent[2] && c[2] < dimensions_[2] - 1) {
                cells[3][0] = grid_[ic + o__0_m1_p1_][0];
                cells[3][1] = grid_[ic + o_p1_m1_p1_][1];
            }
        } else if (1 == adjacent[1] && c[1] < dimensions_[1] - 1) {
            cells[1][0] = grid_[ic + o__0_p1__0_][0];
            cells[1][1] = grid_[ic + o_p1_p1__0_][1];

            if (-1 == adjacent[2] && c[2] > 0) {
                cells[3][0] = grid_[ic + o__0_p1_m1_][0];
                cells[3][1] = grid_[ic + o_p1_p1_m1_][1];
            } else if (1 == adjacent[2] && c[2] < dimensions_[2] - 1) {
                cells[3][0] = grid_[ic + o__0_p1_p1_][0];
                cells[3][1] = grid_[ic + o_p1_p1_p1_][1];
            }
        }

        if (-1 == adjacent[2] && c[2] > 0) {
            cells[2][0] = grid_[ic + o__0__0_m1_][0];
            cells[2][1] = grid_[ic + o_p1__0_m1_][1];
        } else if (1 == adjacent[2] && c[2] < dimensions_[2] - 1) {
            cells[2][0] = grid_[ic + o__0__0_p1_][0];
            cells[2][1] = grid_[ic + o_p1__0_p1_][1];
        }

        return;
    }

    if (-1 == adjacent[1] && c[1] > 0) {
        cells[1] = grid_[ic + o__0_m1__0_];
        if (-1 == adjacent[2] && c[2] > 0) {
            cells[2] = grid_[ic + o__0__0_m1_];
            cells[3] = grid_[ic + o__0_m1_m1_];
        } else if (1 == adjacent[2] && c[2] < dimensions_[2] - 1) {
            cells[2] = grid_[ic + o__0__0_p1_];
            cells[3] = grid_[ic + o__0_m1_p1_];
        }

        return;
    } else if (1 == adjacent[1] && c[1] < dimensions_[1] - 1) {
        cells[1] = grid_[ic + o__0_p1__0_];

        if (-1 == adjacent[2] && c[2] > 0) {
            cells[2] = grid_[ic + o__0__0_m1_];
            cells[3] = grid_[ic + o__0_p1_m1_];
        } else if (1 == adjacent[2] && c[2] < dimensions_[2] - 1) {
            cells[2] = grid_[ic + o__0__0_p1_];
            cells[3] = grid_[ic + o__0_p1_p1_];
        }

        return;
    }

    if (-1 == adjacent[2] && c[2] > 0) {
        cells[2] = grid_[ic + o__0__0_m1_];
    } else if (1 == adjacent[2] && c[2] < dimensions_[2] - 1) {
        cells[2] = grid_[ic + o__0__0_p1_];
    }
}

float3 Grid::scattering_coefficient(Intersection const&  intersection,
                                    scene::Worker const& worker) noexcept {
    using Filter_settings = scene::material::Sampler_settings::Filter;

    auto const& material = *intersection.material();

    if (material.is_heterogeneous_volume()) {
        scene::entity::Composed_transformation temp;
        auto const& transformation = intersection.prop->transformation_at(0.f, temp);

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
