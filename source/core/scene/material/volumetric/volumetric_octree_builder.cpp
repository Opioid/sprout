#include "volumetric_octree_builder.hpp"
#include "base/math/vector4.inl"
#include "base/thread/thread_pool.hpp"
#include "image/texture/texture.hpp"
#include "scene/material/collision_coefficients.inl"

namespace scene::material::volumetric {

Octree_builder::Build_node::~Build_node() {
    for (uint32_t i = 0; i < 8; ++i) {
        delete children[i];
    }
}

void Octree_builder::build(Gridtree& tree, Texture const& texture, CM const& cm,
                           thread::Pool& pool) {
    pool.wait_async();

    int3 const d = texture.dimensions_3();

    int3 num_cells = d >> Gridtree::Log2_cell_dim;

    num_cells += math::min(d - (num_cells << Gridtree::Log2_cell_dim), 1);

    uint32_t const cell_len = static_cast<uint32_t>(num_cells[0] * num_cells[1] * num_cells[2]);

    Build_node* grid = new Build_node[cell_len];

    Splitter* splitters = new Splitter[pool.num_threads()];

    pool.run_range(
        [splitters, grid, &texture, &cm, &num_cells](uint32_t id, int32_t begin, int32_t end) {
            Splitter& splitter = splitters[id];

            int32_t const area = num_cells[0] * num_cells[1];

            for (int32_t i = begin; i < end; ++i) {
                int3 c;
                c[2] = i / area;

                int32_t const t = c[2] * area;

                c[1] = (i - t) / num_cells[0];
                c[0] = i - (t + c[1] * num_cells[0]);

                int3 const min = c << Gridtree::Log2_cell_dim;
                int3 const max = min + Gridtree::Cell_dim;

                Box const box{{min, max}};
                splitter.split(&grid[i], box, texture, cm, 0);
            }
        },
        0, static_cast<int32_t>(cell_len));

    uint32_t num_nodes = cell_len;
    uint32_t num_data  = 0;

    for (uint32_t i = 0, len = pool.num_threads(); i < len; ++i) {
        num_nodes += splitters[i].num_nodes;
        num_data += splitters[i].num_data;
    }

    delete[] splitters;

    tree.set_dimensions(d, num_cells);

    nodes_ = tree.allocate_nodes(num_nodes);

    data_ = tree.allocate_data(num_data);

    uint32_t next = cell_len;
    uint32_t data = 0;

    for (uint32_t i = 0; i < cell_len; ++i) {
        serialize(&grid[i], i, next, data);
    }

    delete[] grid;
}

void Octree_builder::Splitter::split(Build_node* node, Box const& box, Texture const& texture,
                                     CM const& cm, uint32_t depth) {
    static int32_t constexpr w = (Gridtree::Cell_dim >> (Gridtree::Log2_cell_dim - 3)) + 1;

    // Without an epsilon the sampled extinction coefficient can sometimes
    // be a tiny bit larger than the majorant computed here.
    // Supposedly due to floating point imprecision.
    static float constexpr mt_epsilon = 0.f;  // 0.003f;

    int3 const d = texture.dimensions_3();

    // Include 1 additional voxel on each border to account for filtering
    int3 const minb = math::max(box.bounds[0] - 1, 0);
    int3 const maxb = math::min(box.bounds[1] + 1, d);

    if (4 == texture.num_channels()) {
        float const distance = cm.minorant_mu_a;
        float const scattering_factor = cm.majorant_mu_a;

        CM lcm(1.f, 0.f);

        for (int32_t z = minb[2]; z < maxb[2]; ++z) {
            for (int32_t y = minb[1]; y < maxb[1]; ++y) {
                for (int32_t x = minb[0]; x < maxb[0]; ++x) {
                    float4 const color = texture.at_4(x, y, z);

                    lcm.add(color[3] * attenuation(color.xyz(), scattering_factor * color.xyz(), distance));
                }
            }
        }

        //        if (min_density > max_density) {
        //            min_density = 0.f;
        //            max_density = 0.f;
        //        }

        float const diff = std::max(lcm.majorant_mu_a - lcm.minorant_mu_a,
                                    lcm.majorant_mu_s - lcm.minorant_mu_s);

        if (Gridtree::Log2_cell_dim - 3 == depth || diff < 0.1f || math::any_less(maxb - minb, w)) {
            for (uint32_t i = 0; i < 8; ++i) {
                node->children[i] = nullptr;
            }

            auto& data = node->data;

            if (0.f == diff) {
                data.minorant_mu_a = lcm.minorant_mu_a;
                data.minorant_mu_s = lcm.minorant_mu_s;
                data.majorant_mu_a = lcm.majorant_mu_a;
                data.majorant_mu_s = lcm.majorant_mu_s;
            } else {
                data.minorant_mu_a = std::max(lcm.minorant_mu_a - mt_epsilon, 0.f);
                data.minorant_mu_s = std::max(lcm.minorant_mu_s - mt_epsilon, 0.f);
                data.majorant_mu_a = 0.f == lcm.majorant_mu_a ? 0.f
                                                              : lcm.majorant_mu_a + mt_epsilon;
                data.majorant_mu_s = lcm.majorant_mu_s;
            }

            if (!data.is_empty()) {
                ++num_data;
            }

            return;
        }
    } else {
        float min_density = 1.f;
        float max_density = 0.f;

        for (int32_t z = minb[2]; z < maxb[2]; ++z) {
            for (int32_t y = minb[1]; y < maxb[1]; ++y) {
                for (int32_t x = minb[0]; x < maxb[0]; ++x) {
                    float const density = texture.at_1(x, y, z);

                    min_density = std::min(density, min_density);
                    max_density = std::max(density, max_density);
                }
            }
        }

        if (min_density > max_density) {
            min_density = 0.f;
            max_density = 0.f;
        }

        float const minorant_mu_a = min_density * cm.minorant_mu_a;
        float const minorant_mu_s = min_density * cm.minorant_mu_s;
        float const majorant_mu_a = max_density * cm.majorant_mu_a;
        float const majorant_mu_s = max_density * cm.majorant_mu_s;

        float const diff = max_density - min_density;

        if (Gridtree::Log2_cell_dim - 3 == depth || diff < 0.1f || math::any_less(maxb - minb, w)) {
            for (uint32_t i = 0; i < 8; ++i) {
                node->children[i] = nullptr;
            }

            auto& data = node->data;

            if (0.f == diff) {
                data.minorant_mu_a = minorant_mu_a;
                data.minorant_mu_s = minorant_mu_s;
                data.majorant_mu_a = majorant_mu_a;
                data.majorant_mu_s = majorant_mu_s;
            } else {
                data.minorant_mu_a = std::max(minorant_mu_a - mt_epsilon, 0.f);
                data.minorant_mu_s = std::max(minorant_mu_s - mt_epsilon, 0.f);
                data.majorant_mu_a = 0.f == majorant_mu_a ? 0.f : majorant_mu_a + mt_epsilon;
                data.majorant_mu_s = majorant_mu_s;
            }

            if (!node->data.is_empty()) {
                ++num_data;
            }

            return;
        }
    }

    ++depth;

    int3 const half = (box.bounds[1] - box.bounds[0]) >> 1;

    int3 const center = box.bounds[0] + half;

    {
        Box const sub{{box.bounds[0], center}};

        node->children[0] = new Build_node;
        split(node->children[0], sub, texture, cm, depth);
    }

    {
        Box const sub{{int3(center[0], box.bounds[0][1], box.bounds[0][2]),
                       int3(box.bounds[1][0], center[1], center[2])}};

        node->children[1] = new Build_node;
        split(node->children[1], sub, texture, cm, depth);
    }

    {
        Box const sub{{int3(box.bounds[0][0], center[1], box.bounds[0][2]),
                       int3(center[0], box.bounds[1][1], center[2])}};

        node->children[2] = new Build_node;
        split(node->children[2], sub, texture, cm, depth);
    }

    {
        Box const sub{{int3(center[0], center[1], box.bounds[0][2]),
                       int3(box.bounds[1][0], box.bounds[1][1], center[2])}};

        node->children[3] = new Build_node;
        split(node->children[3], sub, texture, cm, depth);
    }

    {
        Box const sub{{int3(box.bounds[0][0], box.bounds[0][1], center[2]),
                       int3(center[0], center[1], box.bounds[1][2])}};

        node->children[4] = new Build_node;
        split(node->children[4], sub, texture, cm, depth);
    }

    {
        Box const sub{{int3(center[0], box.bounds[0][1], center[2]),
                       int3(box.bounds[1][0], center[1], box.bounds[1][2])}};

        node->children[5] = new Build_node;
        split(node->children[5], sub, texture, cm, depth);
    }

    {
        Box const sub{{int3(box.bounds[0][0], center[1], center[2]),
                       int3(center[0], box.bounds[1][1], box.bounds[1][2])}};

        node->children[6] = new Build_node;
        split(node->children[6], sub, texture, cm, depth);
    }

    {
        Box const sub{{center, box.bounds[1]}};

        node->children[7] = new Build_node;
        split(node->children[7], sub, texture, cm, depth);
    }

    num_nodes += 8;
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
