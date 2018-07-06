#include "photon_grid.hpp"
#include <algorithm>
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/thread/thread_pool.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"

namespace rendering::integrator::photon {

Grid::Grid() : num_photons_(0), photons_(nullptr), dimensions_(0), grid_(nullptr) {}

Grid::~Grid() {
    memory::free_aligned(grid_);
}

void Grid::resize(math::AABB const& aabb, float radius, float merge_radius_factor) {
    photon_radius_       = radius;
    inverse_cell_size_   = 1.f / (2.f * radius);
    merge_radius_factor_ = merge_radius_factor;

    min_ = aabb.min();

    int3 const dimensions = map3(aabb.max()) + int3(1);

    int32_t const num_cells = dimensions[0] * dimensions[1] * dimensions[2];

    if (dimensions_ != dimensions) {
        dimensions_ = dimensions;
        max_coords_ = dimensions - int3(1);

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

void Grid::update(uint32_t num_photons, Photon* photons) {
    num_photons_ = num_photons;
    photons_     = photons;

    if (0 == num_photons) {
        return;
    }

    std::sort(photons, photons + num_photons, [this](Photon const& a, Photon const& b) -> bool {
        int32_t const ida = map1(a.p);
        int32_t const idb = map1(b.p);

        return ida < idb;
    });

    int32_t const num_cells = dimensions_[0] * dimensions_[1] * dimensions_[2];
    int32_t const len       = static_cast<int32_t>(num_photons);

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

uint32_t Grid::reduce_and_move(Photon* photons, uint32_t* num_reduced, thread::Pool& pool) {
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

float3 Grid::li(f_float3 position, scene::material::Sample const& sample,
                uint32_t num_paths) const {
    if (0 == num_photons_) {
        return float3::identity();
    }

    float const squared_radius     = photon_radius_ * photon_radius_;
    float const inv_squared_radius = 1.f / squared_radius;

    float const f = 1.f / (static_cast<float>(num_paths) * squared_radius);

    float3 result = float3::identity();

    /*
            for (uint32_t i = 0, len = num_photons_; i < len; ++i) {
                auto const& photon           = photons_[i];

                float const squared_distance = math::squared_distance(photon.p, position);
                if (squared_distance <= squared_radius) {
                    auto const bxdf = sample.evaluate(photon.wi);

                    float const n_dot_wi = sample.base_layer().abs_n_dot(photon.wi);

                    if (n_dot_wi > 0.f) {
                        float const clamped_n_dot_wi = scene::material::clamp(n_dot_wi);

                        float const k = kernel(squared_distance, inv_squared_radius);

                        result += (k * f) / clamped_n_dot_wi * photon.alpha * bxdf.reflection;
                    }
                }
            }
    */

    int2 cells[4];
    adjacent_cells(position, cells);

    for (uint32_t c = 0; c < 4; ++c) {
        int2 const cell = cells[c];

        for (int32_t i = cell[0], len = cell[1]; i < len; ++i) {
            auto const& photon = photons_[i];

            //            if (math::dot(photon.n, sample.geometric_normal()) < 0.5f) {
            //                continue;
            //            }

            if (float const squared_distance = math::squared_distance(photon.p, position);
                squared_distance <= squared_radius) {
                if (float const n_dot_wi = sample.base_layer().abs_n_dot(photon.wi);
                    n_dot_wi > 0.f) {
                    float const clamped_n_dot_wi = scene::material::clamp(n_dot_wi);

                    float const k = kernel(squared_distance, inv_squared_radius);

                    auto const bxdf = sample.evaluate(photon.wi);

                    result += (k * f) / clamped_n_dot_wi * float3(photon.alpha) * bxdf.reflection;
                }
            }
        }
    }

    return result;
}

size_t Grid::num_bytes() const {
    int32_t const num_cells = dimensions_[0] * dimensions_[1] * dimensions_[2];

    size_t const num_bytes = static_cast<uint32_t>(num_cells) * sizeof(int2);

    return num_bytes;
}

uint32_t Grid::reduce(int32_t begin, int32_t end) {
    float const merge_distance = math::pow2(merge_radius_factor_ * photon_radius_);

    uint32_t num_reduced = 0;

    for (int32_t i = begin, ilen = end; i < ilen; ++i) {
        auto& pa = photons_[i];

        if (pa.alpha[0] < 0.f) {
            continue;
        }

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

                //                if (math::dot(pa.n, pb.n) < 0.5f) {
                //                    continue;
                //                }

                if (math::squared_distance(pa.p, pb.p) < merge_distance) {
                    float3 const sum = float3(pa.alpha) + float3(pb.alpha);
                    pa.alpha[0]      = sum[0];
                    pa.alpha[1]      = sum[1];
                    pa.alpha[2]      = sum[2];

                    pb.alpha[0] = -1.f;
                    ++num_reduced;
                }
            }
        }
    }

    return num_reduced;
}

int32_t Grid::map1(f_float3 v) const {
    int3 const c = static_cast<int3>(inverse_cell_size_ * (v - min_));

    return (c[2] * dimensions_[1] + c[1]) * dimensions_[0] + c[0];
}

int3 Grid::map3(f_float3 v) const {
    return static_cast<int3>(inverse_cell_size_ * (v - min_));
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

int3 Grid::map3(f_float3 v, int8_t adjacent[3]) const {
    float3 const r = inverse_cell_size_ * (v - min_);

    int3 const c = static_cast<int3>(r);

    float3 const d = r - static_cast<float3>(c);

    adjacent[0] = photon::adjacent(d[0]);
    adjacent[1] = photon::adjacent(d[1]);
    adjacent[2] = photon::adjacent(d[2]);

    return c;
}

void Grid::adjacent_cells(f_float3 v, int2 cells[4]) const {
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

}  // namespace rendering::integrator::photon