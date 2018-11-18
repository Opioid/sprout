#include "volumetric_octree_builder.hpp"
#include "base/math/vector3.inl"
#include "image/texture/texture.hpp"
#include "scene/material/collision_coefficients.inl"

#include <iostream>
#include "base/math/print.hpp"

namespace scene::material::volumetric {

Octree_builder::Build_node::~Build_node() {
    for (uint32_t i = 0; i < 8; ++i) {
        delete children[i];
    }
}

void Octree_builder::build(Gridtree& tree, Texture const& texture, CM const& idata) {
    int3 const d = texture.dimensions_3();

    std::cout << "voxels: " << d << std::endl;

    int3 num_cells = d / Gridtree::Cell_dim;

    num_cells += math::min(d - Gridtree::Cell_dim * num_cells, 1);

    uint32_t const cell_len = static_cast<uint32_t>(num_cells[0] * num_cells[1] * num_cells[2]);

    num_nodes_                 = cell_len;
    num_data_                  = 0;
    num_homogeneous_non_empty_ = 0;

    Build_node* grid = new Build_node[cell_len];

    Build_node* node = grid;
    for (int32_t z = 0; z < num_cells[2]; ++z) {
        for (int32_t y = 0; y < num_cells[1]; ++y) {
            for (int32_t x = 0; x < num_cells[0]; ++x, ++node) {
                int3 const min = Gridtree::Cell_dim * int3(x, y, z);
                int3 const max = min + Gridtree::Cell_dim;

                Box const box{{min, max}};
                split(node, box, texture, idata, 0);
            }
        }
    }

    tree.set_dimensions(d, num_cells);

    nodes_ = tree.allocate_nodes(num_nodes_);

    data_ = tree.allocate_data(num_data_);

    uint32_t next = cell_len;
    uint32_t data = 0;

    for (uint32_t i = 0; i < cell_len; ++i) {
        serialize(&grid[i], i, next, data);
    }

    delete[] grid;

    std::cout << "#Nodes: " << num_nodes_ << " #Data: " << num_data_ << std::endl;

    std::cout << "% homogeneous: "
              << static_cast<float>(num_homogeneous_non_empty_) / static_cast<float>(num_data_)
              << std::endl;
}

enum Adjacent { None = 0, Positive = 1, Negative = 2 };

static inline uint8_t adjacent(int32_t x, int32_t lower, int32_t upper, int32_t limit) noexcept {
    if (x < lower && lower < limit) {
        return Negative;
    }

    if (x >= upper) {
        return Positive;
    }

    return None;
}

static inline uint8_t adjacent(int3 const& v, Box const& box, int3 const& limit) noexcept {
    uint8_t a = static_cast<uint8_t>(adjacent(v[0], box.bounds[0][0], box.bounds[1][0], limit[0])
                                     << 4);
    a |= static_cast<uint8_t>(adjacent(v[1], box.bounds[0][1], box.bounds[1][1], limit[1]) << 2);
    a |= adjacent(v[2], box.bounds[0][2], box.bounds[1][2], limit[2]);

    return a;
}

static inline float filtered_density(int32_t x, int32_t y, int32_t z, Box const& box,
                                     image::texture::Texture const& texture) noexcept {
    int3 const d = texture.dimensions_3();

    uint8_t const a = adjacent(int3(x, y, z), box, d);

    float const center = texture.at_1(x, y, z);

    if (2 == a) {
        return 0.5f * (texture.at_1(x, y, z + 1) + center);
    } else if (8 == a) {
        return 0.5f * (texture.at_1(x, y + 1, z) + center);
    } else if (10 == a) {
        float const t0 = texture.at_1(x, y + 1, z);
        float const t1 = texture.at_1(x, y, z + 1);
        float const d0 = 0.5f * (t0 + center);
        float const d1 = 0.5f * (t1 + center);
        float const d2 = 0.5f * (t0 + t1);
        return std::max(d0, std::max(d1, d2));
    } else if (32 == a) {
        return 0.5f * (texture.at_1(x + 1, y, z) + center);
    } else if (34 == a) {
        float const t0 = texture.at_1(x + 1, y, z);
        float const t1 = texture.at_1(x, y, z + 1);
        float const d0 = 0.5f * (t0 + center);
        float const d1 = 0.5f * (t1 + center);
        float const d2 = 0.5f * (t0 + t1);
        return std::max(d0, std::max(d1, d2));
    } else if (40 == a) {
        float const t0 = texture.at_1(x + 1, y, z);
        float const t1 = texture.at_1(x, y + 1, z);
        float const d0 = 0.5f * (t0 + center);
        float const d1 = 0.5f * (t1 + center);
        float const d2 = 0.5f * (t0 + t1);
        return std::max(d0, std::max(d1, d2));
        /*   } else if (42 == a) {
               float const d0 = 0.5f * (texture.at_1(x + 1, y, z) + texture.at_1(x, y, z));
               float const d1 = 0.5f * (texture.at_1(x, y + 1, z) + texture.at_1(x, y, z));
               float const d2 = 0.5f * (texture.at_1(x, y, z + 1) + texture.at_1(x, y, z));
               float const d3 = 0.5f * (texture.at_1(x + 1, y + 1, z) + texture.at_1(x, y, z));
               float const d4 = 0.5f * (texture.at_1(x, y + 1, z + 1) + texture.at_1(x, y, z));
               float const d5 = 0.5f * (texture.at_1(x + 1, y, z + 1) + texture.at_1(x, y, z));
               float const d6 = 0.5f * (texture.at_1(x + 1, y + 1, z + 1) + texture.at_1(x, y, z));
               density        = std::max(d0, std::max(d1, std::max(d2, std::max(d3, std::max(d4,
           std::max(d5, d6))))));
                       */
    }

    return center;
}

void Octree_builder::split(Build_node* node, Box const& box, Texture const& texture,
                           CM const& idata, uint32_t depth) {
    int3 const d = texture.dimensions_3();

    float min_density = 1.f;
    float max_density = 0.f;

    // Include 1 additional voxel on each border to account for filtering
    int3 const minb = math::max(box.bounds[0] - 1, 0);
    int3 const maxb = math::min(box.bounds[1] + 1, d);

    static bool constexpr imensity = true;

    if (imensity) {
        Box const bb{{math::max(box.bounds[0], 0), math::min(box.bounds[1], d)}};

        for (int32_t z = minb[2]; z < maxb[2]; ++z) {
            for (int32_t y = minb[1]; y < maxb[1]; ++y) {
                for (int32_t x = minb[0]; x < maxb[0]; ++x) {
                    float const density = filtered_density(x, y, z, bb, texture);

                    min_density = std::min(density, min_density);
                    max_density = std::max(density, max_density);
                }
            }
        }
    } else {
        for (int32_t z = minb[2]; z < maxb[2]; ++z) {
            for (int32_t y = minb[1]; y < maxb[1]; ++y) {
                for (int32_t x = minb[0]; x < maxb[0]; ++x) {
                    float const density = texture.at_1(x, y, z);

                    min_density = std::min(density, min_density);
                    max_density = std::max(density, max_density);
                }
            }
        }
    }

    if (min_density > max_density) {
        min_density = 0.f;
        max_density = 0.f;
    }

    float const minorant_mu_a = min_density * idata.minorant_mu_a;
    float const minorant_mu_s = min_density * idata.minorant_mu_s;
    float const majorant_mu_a = max_density * idata.majorant_mu_a;
    float const majorant_mu_s = max_density * idata.majorant_mu_s;

    float const diff = (majorant_mu_a + majorant_mu_s) - (minorant_mu_a + minorant_mu_s);

    int3 const half = (box.bounds[1] - box.bounds[0]) >> 1;

    static int32_t constexpr w = (Gridtree::Cell_dim >> (Gridtree::Log2_cell_dim - 2)) + 1;

    if (Gridtree::Log2_cell_dim - 2 == depth || diff < 0.1f || math::any_less(maxb - minb, w)) {
        for (uint32_t i = 0; i < 8; ++i) {
            node->children[i] = nullptr;
        }

        if (0.f == diff) {
            node->data.minorant_mu_a = minorant_mu_a;
            node->data.minorant_mu_s = minorant_mu_s;
            node->data.majorant_mu_a = majorant_mu_a;
            node->data.majorant_mu_s = majorant_mu_s;
        } else {
            // Without an epsilon the sampled extinction coefficient can sometimes
            // be a tiny bit larger than the majorant computed here.
            // Supposedly due to floating point imprecision.
            static float constexpr mt_epsilon = 0.005f;

            node->data.minorant_mu_a = std::max(minorant_mu_a - mt_epsilon, 0.f);
            node->data.minorant_mu_s = std::max(minorant_mu_s - mt_epsilon, 0.f);
            node->data.majorant_mu_a = 0.f == majorant_mu_a ? 0.f : majorant_mu_a + mt_epsilon;
            node->data.majorant_mu_s = majorant_mu_s;
        }

        if (!node->data.is_empty()) {
            ++num_data_;

            if (min_density == max_density) {
                ++num_homogeneous_non_empty_;
            }
        }

        return;
    }

    ++depth;

    int3 const center = box.bounds[0] + half;

    {
        Box const sub{{box.bounds[0], center}};

        node->children[0] = new Build_node;
        split(node->children[0], sub, texture, idata, depth);
    }

    {
        Box const sub{{int3(center[0], box.bounds[0][1], box.bounds[0][2]),
                       int3(box.bounds[1][0], center[1], center[2])}};

        node->children[1] = new Build_node;
        split(node->children[1], sub, texture, idata, depth);
    }

    {
        Box const sub{{int3(box.bounds[0][0], center[1], box.bounds[0][2]),
                       int3(center[0], box.bounds[1][1], center[2])}};

        node->children[2] = new Build_node;
        split(node->children[2], sub, texture, idata, depth);
    }

    {
        Box const sub{{int3(center[0], center[1], box.bounds[0][2]),
                       int3(box.bounds[1][0], box.bounds[1][1], center[2])}};

        node->children[3] = new Build_node;
        split(node->children[3], sub, texture, idata, depth);
    }

    {
        Box const sub{{int3(box.bounds[0][0], box.bounds[0][1], center[2]),
                       int3(center[0], center[1], box.bounds[1][2])}};

        node->children[4] = new Build_node;
        split(node->children[4], sub, texture, idata, depth);
    }

    {
        Box const sub{{int3(center[0], box.bounds[0][1], center[2]),
                       int3(box.bounds[1][0], center[1], box.bounds[1][2])}};

        node->children[5] = new Build_node;
        split(node->children[5], sub, texture, idata, depth);
    }

    {
        Box const sub{{int3(box.bounds[0][0], center[1], center[2]),
                       int3(center[0], box.bounds[1][1], box.bounds[1][2])}};

        node->children[6] = new Build_node;
        split(node->children[6], sub, texture, idata, depth);
    }

    {
        Box const sub{{center, box.bounds[1]}};

        node->children[7] = new Build_node;
        split(node->children[7], sub, texture, idata, depth);
    }

    num_nodes_ += 8;
}

void Octree_builder::serialize(Build_node* node, uint32_t current, uint32_t& next, uint32_t& data) {
    auto& n = nodes_[current];

    if (node->children[0]) {
        n.set_children(next);

        current = next;
        next += 8;

        for (uint32_t i = 0; i < 8; ++i) {
            serialize(node->children[i], current + i, next, data);
        }
    } else if (!node->data.is_empty()) {
        n.set_data(data);

        data_[data] = node->data;

        ++data;
    } else {
        n.set_empty();
    }
}

}  // namespace scene::material::volumetric
