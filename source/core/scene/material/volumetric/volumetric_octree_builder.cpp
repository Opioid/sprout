#include "volumetric_octree_builder.hpp"
#include "base/math/vector3.inl"
#include "image/texture/texture.hpp"

#include <iostream>
#include "base/math/print.hpp"

namespace scene::material::volumetric {

Octree_builder::Build_node::~Build_node() {
    for (uint32_t i = 0; i < 8; ++i) {
        delete children[i];
    }
}

void Octree_builder::build(Gridtree& tree, image::texture::Texture const& texture,
                           float2 min_max_extinction) {
    int3 const d = texture.dimensions_3();

    int32_t const cd   = 36;
    int3 const    cell = math::min(d, cd);

    int3 num_cells = d / cell;

    num_cells += math::min(d - num_cells * cell, 1);

    int32_t const cell_len = num_cells[0] * num_cells[1] * num_cells[2];

    num_nodes_ = cell_len;

    Build_node* grid = new Build_node[static_cast<size_t>(cell_len)];

    Build_node* node = grid;
    for (int32_t z = 0; z < num_cells[2]; ++z) {
        for (int32_t y = 0; y < num_cells[1]; ++y) {
            for (int32_t x = 0; x < num_cells[0]; ++x, ++node) {
                int3 const min = int3(x, y, z) * cell;
                int3 const max = math::min(min + cell, d);
                Box const  box{{min, max}};
                split(node, box, texture, min_max_extinction, 0, 2);
            }
        }
    }

    tree.set_dimensions(d, cell, num_cells);

    nodes_ = tree.allocate_nodes(num_nodes_);

    int32_t next = cell_len;

    for (int32_t i = 0; i < cell_len; ++i) {
        serialize(&grid[i], i, next);
    }

    delete[] grid;
}

void Octree_builder::split(Build_node* node, Box const& box, image::texture::Texture const& texture,
                           float2 min_max_extinction, uint32_t depth, uint32_t max_depth) {
    // Include 1 additional voxel on each border to account for filtering
    int3 const minb = math::max(box.bounds[0] - 1, 0);
    int3 const maxb = math::min(box.bounds[1] + 1, texture.dimensions_3());

    float min_density = 1.f;
    float max_density = 0.f;
    for (int32_t z = minb[2], mz = maxb[2]; z < mz; ++z) {
        for (int32_t y = minb[1], my = maxb[1]; y < my; ++y) {
            for (int32_t x = minb[0], mx = maxb[0]; x < mx; ++x) {
                float const density = texture.at_1(x, y, z);

                min_density = std::min(density, min_density);
                max_density = std::max(density, max_density);
            }
        }
    }

    // Without this epsilon the sampled extinction coefficient is sometimes
    // a tiny bit larger than the majorant computed here
    static float constexpr mt_epsilon = 0.01f;

    float const minorant = min_density * min_max_extinction[0];
    float const majorant = max_density * min_max_extinction[1];

    float const diff = majorant - minorant;

    bool const homogeneous = 0 == diff;

    if (homogeneous) {
        node->minorant_mu_t = minorant;
        node->majorant_mu_t = majorant;
    } else {
        node->minorant_mu_t = std::max(minorant - mt_epsilon, 0.f);
        node->majorant_mu_t = 0.f == majorant ? 0.f : majorant + mt_epsilon;
    }

    int3 const half = (box.bounds[1] - box.bounds[0]) / 2;

    if (max_depth == depth || homogeneous || math::any_lesser(half, 3)) {
        for (uint32_t i = 0; i < 8; ++i) {
            node->children[i] = nullptr;
        }

        //        if (min_density == max_density) {
        //            std::cout << min_density << std::endl;
        //        }

        return;
    }

    ++depth;

    int3 const center = box.bounds[0] + half;

    {
        Box const sub{{box.bounds[0], center}};

        node->children[0] = new Build_node;
        split(node->children[0], sub, texture, min_max_extinction, depth, max_depth);
    }

    {
        Box const sub{{int3(center[0], box.bounds[0][1], box.bounds[0][2]),
                       int3(box.bounds[1][0], center[1], center[2])}};

        node->children[1] = new Build_node;
        split(node->children[1], sub, texture, min_max_extinction, depth, max_depth);
    }

    {
        Box const sub{{int3(box.bounds[0][0], center[1], box.bounds[0][2]),
                       int3(center[0], box.bounds[1][1], center[2])}};

        node->children[2] = new Build_node;
        split(node->children[2], sub, texture, min_max_extinction, depth, max_depth);
    }

    {
        Box const sub{{int3(center[0], box.bounds[0][1] + half[1], box.bounds[0][2]),
                       int3(box.bounds[1][0], box.bounds[1][1], center[2])}};

        node->children[3] = new Build_node;
        split(node->children[3], sub, texture, min_max_extinction, depth, max_depth);
    }

    {
        Box const sub{{int3(box.bounds[0][0], box.bounds[0][1], center[2]),
                       int3(center[0], center[1], box.bounds[1][2])}};

        node->children[4] = new Build_node;
        split(node->children[4], sub, texture, min_max_extinction, depth, max_depth);
    }

    {
        Box const sub{{int3(center[0], box.bounds[0][1], center[2]),
                       int3(box.bounds[1][0], center[1], box.bounds[1][2])}};

        node->children[5] = new Build_node;
        split(node->children[5], sub, texture, min_max_extinction, depth, max_depth);
    }

    {
        Box const sub{{int3(box.bounds[0][0], center[1], center[2]),
                       int3(center[0], box.bounds[1][1], box.bounds[1][2])}};

        node->children[6] = new Build_node;
        split(node->children[6], sub, texture, min_max_extinction, depth, max_depth);
    }

    {
        Box const sub{{center, box.bounds[1]}};

        node->children[7] = new Build_node;
        split(node->children[7], sub, texture, min_max_extinction, depth, max_depth);
    }

    num_nodes_ += 8;
}

void Octree_builder::serialize(Build_node* node, int32_t current, int32_t& next) {
    auto& n = nodes_[current];

    if (node->children[0]) {
        n.children      = next;
        n.minorant_mu_t = node->minorant_mu_t;
        n.majorant_mu_t = node->majorant_mu_t;

        current = next;
        next += 8;

        for (int32_t i = 0; i < 8; ++i) {
            serialize(node->children[i], current + i, next);
        }
    } else {
        n.children      = 0;
        n.minorant_mu_t = node->minorant_mu_t;
        n.majorant_mu_t = node->majorant_mu_t;
    }
}

}  // namespace scene::material::volumetric
