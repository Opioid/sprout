#include "photon_map.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/thread/thread_pool.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"

#include <iostream>
#include "base/math/print.hpp"

namespace rendering::integrator::photon {

Map::Map(uint32_t num_photons, float photon_radius, uint32_t num_workers)
    : num_photons_(num_photons),
      photons_(nullptr),
      photon_radius_(photon_radius),
      inverse_cell_size_(1.f / (2.5f * photon_radius)),
      grid_dimensions_(0),
      grid_(nullptr),
      num_reduced_(new uint32_t[num_workers]) {}

Map::~Map() {
    delete[] num_reduced_;
    delete[] grid_;
    delete[] photons_;
}

void Map::prepare() {
    delete[] photons_;
    photons_ = new Photon[num_photons_];
}

void Map::insert(Photon const& photon, uint32_t index) {
    photons_[index] = photon;
}

static inline int3 map(f_float3 v, f_float3 min, float inverse_cell) {
    return static_cast<int3>(inverse_cell * (v - min));
}

static inline int8_t adjacent(float s) {
    if (s <= 0.4f) {
        return -1;
    }

    if (s >= 0.6f) {
        return 1;
    }

    return 0;
}

static inline int3 map(f_float3 v, f_float3 min, float inverse_cell, int8_t adjacent[3]) {
    float3 const r = inverse_cell * (v - min);

    int3 const c = static_cast<int3>(r);

    float3 const d = r - static_cast<float3>(c);

    adjacent[0] = photon::adjacent(d[0]);
    adjacent[1] = photon::adjacent(d[1]);
    adjacent[2] = photon::adjacent(d[2]);

    return c;
}

static inline int32_t map(f_float3 v, f_float3 min, int32_t area, int32_t width,
                          float inverse_cell) {
    int3 const c = static_cast<int3>(inverse_cell * (v - min));

    return c[2] * area + c[1] * width + c[0];
}

void Map::compile(uint32_t num_paths, math::AABB const& aabb, thread::Pool& pool) {
    num_paths_ = num_paths;

    aabb_ = aabb;

    int3 const grid_dimensions = photon::map(aabb.max(), aabb.min(), inverse_cell_size_) + int3(1);

    int32_t const num_cells = grid_dimensions[0] * grid_dimensions[1] * grid_dimensions[2];

    if (grid_dimensions_ != grid_dimensions) {
        grid_dimensions_ = grid_dimensions;

        grid_area_ = grid_dimensions[0] * grid_dimensions[1];

        delete[] grid_;
        grid_ = new int2[static_cast<uint32_t>(num_cells)];

        o__0_m1__0_ = -grid_dimensions[0];
        o__0_p1__0_ = +grid_dimensions[0];

        o__0__0_m1_ = -grid_area_;
        o__0__0_p1_ = +grid_area_;

        o_m1_m1__0_ = -1 - grid_dimensions[0];
        o_m1_p1__0_ = -1 + grid_dimensions[0];
        o_p1_m1__0_ = +1 - grid_dimensions[0];
        o_p1_p1__0_ = +1 + grid_dimensions[0];

        o_m1_m1_m1_ = -1 - grid_dimensions[0] - grid_area_;
        o_m1_m1_p1_ = -1 - grid_dimensions[0] + grid_area_;
        o_m1_p1_m1_ = -1 + grid_dimensions[0] - grid_area_;
        o_m1_p1_p1_ = -1 + grid_dimensions[0] + grid_area_;

        o_p1_m1_m1_ = +1 - grid_dimensions[0] - grid_area_;
        o_p1_m1_p1_ = +1 - grid_dimensions[0] + grid_area_;
        o_p1_p1_m1_ = +1 + grid_dimensions[0] - grid_area_;
        o_p1_p1_p1_ = +1 + grid_dimensions[0] + grid_area_;

        o_m1__0_m1_ = -1 - grid_area_;
        o_m1__0_p1_ = -1 + grid_area_;
        o_p1__0_m1_ = +1 - grid_area_;
        o_p1__0_p1_ = +1 + grid_area_;

        o__0_m1_m1_ = -grid_dimensions[0] - grid_area_;
        o__0_m1_p1_ = -grid_dimensions[0] + grid_area_;
        o__0_p1_m1_ = +grid_dimensions[0] - grid_area_;
        o__0_p1_p1_ = +grid_dimensions[0] + grid_area_;
    }

    update_grid(true);

    // return;

    pool.run_range(
        [this](uint32_t id, int32_t begin, int32_t end) { num_reduced_[id] = reduce(begin, end); },
        0, num_photons_);

    uint32_t comp_num_photons = num_photons_;

    for (uint32_t i = 0, len = pool.num_threads(); i < len; ++i) {
        comp_num_photons -= num_reduced_[i];
    }

    //    uint32_t const num_reduced = reduce(0, num_photons_);
    //    uint32_t const comp_num_photons = num_photons_ - num_reduced;

    float const percentage = static_cast<float>(comp_num_photons) /
                             static_cast<float>(num_photons_);

    std::cout << comp_num_photons << " left of " << num_photons_ << " ("
              << static_cast<uint32_t>(100.f * percentage) << "%)" << std::endl;

    Photon* comp_photons = new Photon[comp_num_photons];

    for (uint32_t i = 0, j = 0, len = num_photons_; i < len; ++i) {
        if (photons_[i].alpha[0] >= 0.f) {
            comp_photons[j++] = photons_[i];
        }
    }

    delete[] photons_;
    photons_     = comp_photons;
    num_photons_ = comp_num_photons;

    update_grid(false);
}

static inline float kernel(float squared_distance, float inv_squared_radius) {
    float const s = 1.f - squared_distance * inv_squared_radius;
    return (3.f * math::Pi_inv) * (s * s);
}

float3 Map::li(f_float3 position, scene::material::Sample const& sample) const {
    float const squared_radius     = photon_radius_ * photon_radius_;
    float const inv_squared_radius = 1.f / squared_radius;

    float const num_paths = static_cast<float>(num_paths_);

    float const f = 1.f / (num_paths * squared_radius);

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

            if (float const squared_distance = math::squared_distance(photon.p, position);
                squared_distance <= squared_radius) {
                if (float const n_dot_wi = sample.base_layer().abs_n_dot(photon.wi);
                    n_dot_wi > 0.f) {
                    float const clamped_n_dot_wi = scene::material::clamp(n_dot_wi);

                    float const k = kernel(squared_distance, inv_squared_radius);

                    auto const bxdf = sample.evaluate(photon.wi);

                    result += (k * f) / clamped_n_dot_wi * photon.alpha * bxdf.reflection;
                }
            }
        }
    }

    return result;
}

size_t Map::num_bytes() const {
    size_t num_bytes = num_photons_ * sizeof(Photon);

    int32_t const num_cells = grid_dimensions_[0] * grid_dimensions_[1] * grid_dimensions_[2];

    num_bytes += static_cast<uint32_t>(num_cells) * sizeof(int2);

    return num_bytes;
}

void Map::update_grid(bool needs_sorting) {
    if (needs_sorting) {
        std::sort(photons_, photons_ + num_photons_,
                  [this](Photon const& a, Photon const& b) -> bool {
                      int32_t const ida = map(a.p);
                      int32_t const idb = map(b.p);

                      return ida < idb;
                  });
    }

    int32_t const num_cells = grid_dimensions_[0] * grid_dimensions_[1] * grid_dimensions_[2];

    int32_t current = 0;
    for (int32_t c = 0; c < num_cells; ++c) {
        int32_t const begin = current;
        for (int32_t len = static_cast<int32_t>(num_photons_); current < len; ++current) {
            if (map(photons_[current].p) != c) {
                break;
            }
        }

        grid_[c][0] = begin;
        grid_[c][1] = current;
    }
}

uint32_t Map::reduce(int32_t begin, int32_t end) {
    float const merge_distance = math::pow2(0.125f * photon_radius_);

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

                if (math::squared_distance(pa.p, pb.p) < merge_distance) {
                    pa.alpha += pb.alpha;
                    pb.alpha = float3(-1.f);
                    ++num_reduced;
                }
            }
        }
    }

    return num_reduced;
}

int32_t Map::map(f_float3 v) const {
    return photon::map(v, aabb_.min(), grid_area_, grid_dimensions_[0], inverse_cell_size_);
}

int3 Map::map(f_float3 v, int8_t adjacent[3]) const {
    return photon::map(v, aabb_.min(), inverse_cell_size_, adjacent);
}

int32_t Map::map(f_int3 c) const {
    return c[2] * grid_area_ + c[1] * grid_dimensions_[0] + c[0];
}

void Map::adjacent_cells(f_float3 v, int2 cells[4]) const {
    int8_t adjacent[3];

    int3 const c = map(v, adjacent);

    int32_t const ic = map(c);

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
            } else if (1 == adjacent[2] && c[2] < grid_dimensions_[2] - 1) {
                cells[3][0] = grid_[ic + o_m1_m1_p1_][0];
                cells[3][1] = grid_[ic + o__0_m1_p1_][1];
            }
        } else if (1 == adjacent[1] && c[1] < grid_dimensions_[1] - 1) {
            cells[1][0] = grid_[ic + o_m1_p1__0_][0];
            cells[1][1] = grid_[ic + o__0_p1__0_][1];

            if (-1 == adjacent[2] && c[2] > 0) {
                cells[3][0] = grid_[ic + o_m1_p1_m1_][0];
                cells[3][1] = grid_[ic + o__0_p1_m1_][1];
            } else if (1 == adjacent[2] && c[2] < grid_dimensions_[2] - 1) {
                cells[3][0] = grid_[ic + o_m1_p1_p1_][0];
                cells[3][1] = grid_[ic + o__0_p1_p1_][1];
            }
        }

        if (-1 == adjacent[2] && c[2] > 0) {
            cells[2][0] = grid_[ic + o_m1__0_m1_][0];
            cells[2][1] = grid_[ic + o__0__0_m1_][1];
        } else if (1 == adjacent[2] && c[2] < grid_dimensions_[2] - 1) {
            cells[2][0] = grid_[ic + o_m1__0_p1_][0];
            cells[2][1] = grid_[ic + o__0__0_p1_][1];
        }

        return;
    } else if (1 == adjacent[0] && c[0] < grid_dimensions_[0] - 1) {
        cells[0][1] = grid_[ic + o_p1__0__0_][1];

        if (-1 == adjacent[1] && c[1] > 0) {
            cells[1][0] = grid_[ic + o__0_m1__0_][0];
            cells[1][1] = grid_[ic + o_p1_m1__0_][1];

            if (-1 == adjacent[2] && c[2] > 0) {
                cells[3][0] = grid_[ic + o__0_m1_m1_][0];
                cells[3][1] = grid_[ic + o_p1_m1_m1_][1];
            } else if (1 == adjacent[2] && c[2] < grid_dimensions_[2] - 1) {
                cells[3][0] = grid_[ic + o__0_m1_p1_][0];
                cells[3][1] = grid_[ic + o_p1_m1_p1_][1];
            }
        } else if (1 == adjacent[1] && c[1] < grid_dimensions_[1] - 1) {
            cells[1][0] = grid_[ic + o__0_p1__0_][0];
            cells[1][1] = grid_[ic + o_p1_p1__0_][1];

            if (-1 == adjacent[2] && c[2] > 0) {
                cells[3][0] = grid_[ic + o__0_p1_m1_][0];
                cells[3][1] = grid_[ic + o_p1_p1_m1_][1];
            } else if (1 == adjacent[2] && c[2] < grid_dimensions_[2] - 1) {
                cells[3][0] = grid_[ic + o__0_p1_p1_][0];
                cells[3][1] = grid_[ic + o_p1_p1_p1_][1];
            }
        }

        if (-1 == adjacent[2] && c[2] > 0) {
            cells[2][0] = grid_[ic + o__0__0_m1_][0];
            cells[2][1] = grid_[ic + o_p1__0_m1_][1];
        } else if (1 == adjacent[2] && c[2] < grid_dimensions_[2] - 1) {
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
        } else if (1 == adjacent[2] && c[2] < grid_dimensions_[2] - 1) {
            cells[2] = grid_[ic + o__0__0_p1_];
            cells[3] = grid_[ic + o__0_m1_p1_];
        }

        return;
    } else if (1 == adjacent[1] && c[1] < grid_dimensions_[1] - 1) {
        cells[1] = grid_[ic + o__0_p1__0_];

        if (-1 == adjacent[2] && c[2] > 0) {
            cells[2] = grid_[ic + o__0__0_m1_];
            cells[3] = grid_[ic + o__0_p1_m1_];
        } else if (1 == adjacent[2] && c[2] < grid_dimensions_[2] - 1) {
            cells[2] = grid_[ic + o__0__0_p1_];
            cells[3] = grid_[ic + o__0_p1_p1_];
        }

        return;
    }

    if (-1 == adjacent[2] && c[2] > 0) {
        cells[2] = grid_[ic + o__0__0_m1_];
    } else if (1 == adjacent[2] && c[2] < grid_dimensions_[2] - 1) {
        cells[2] = grid_[ic + o__0__0_p1_];
    }
}

}  // namespace rendering::integrator::photon
