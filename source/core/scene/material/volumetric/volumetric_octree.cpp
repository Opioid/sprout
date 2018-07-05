#include "volumetric_octree.hpp"
#include "base/math/aabb.inl"
#include "base/math/ray.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"

#include "base/debug/assert.hpp"

#include <iostream>
#include "base/math/print.hpp"

namespace scene::material::volumetric {

Gridtree::Gridtree() : num_nodes_(0), nodes_(nullptr) {}

Gridtree::~Gridtree() {
    memory::free_aligned(nodes_);
}

Node* Gridtree::allocate_nodes(uint32_t num_nodes) {
    if (num_nodes != num_nodes_) {
        num_nodes_ = num_nodes;

        memory::free_aligned(nodes_);
        nodes_ = memory::allocate_aligned<Node>(num_nodes);
    }

    return nodes_;
}

void Gridtree::set_dimensions(int3 const& dimensions, int3 const& cell_dimensions,
                              int3 const& num_cells) {
    dimensions_ = dimensions;

    num_cells_ = num_cells;

    std::cout << num_cells << " -> ";

    cell_dimensions_ = float3(cell_dimensions) / float3(dimensions);

    //	std::cout << cell_dimensions_ << std::endl;

    //	cell_dimensions_ = float3(cell_dimensions) / float3(cell_dimensions * num_cells);

    std::cout << cell_dimensions_ << std::endl;

    //	factor_ = float3(num_cells) / (float3(dimensions) / float3(cell_dimensions));

    factor_ = float3(dimensions) / float3(cell_dimensions * num_cells);

    std::cout << factor_ << std::endl;
}

bool Gridtree::is_valid() const {
    return nullptr != nodes_;
}

bool Gridtree::intersect(math::Ray& ray, float& majorant_mu_t) const {
    math::AABB box(float3(0.f), float3(1.f));

    float3 p = ray.point(ray.min_t);

    if (math::any_lesser_equal(p, 0.f) || math::any_greater_equal(p, 1.f)) {
        float hit_t;
        if (!box.intersect_p(ray, hit_t)) {
            return false;
        }

        ray.min_t = hit_t;
        p         = ray.point(ray.min_t);
    }

    int3 const v = int3(float3(num_cells_) * (factor_ * p));

    if (math::any_greater_equal(v, num_cells_)) {
        return false;
    }

    uint32_t index = v[2] * (num_cells_[0] * num_cells_[1]) + v[1] * num_cells_[0] + v[0];

    box.bounds[0] = float3(v) * cell_dimensions_;
    box.bounds[1] = math::min(box.bounds[0] + cell_dimensions_, 1.f);

    for (;;) {
        uint32_t const children = nodes_[index].children;

        if (0 == children) {
            break;
        }

        index = children;

        float3 const half = box.halfsize();

        float3 const center = box.bounds[0] + half;

        if (p[0] < center[0]) {
            box.bounds[1][0] = center[0];
        } else {
            box.bounds[0][0] = center[0];
            index += 1;
        }

        if (p[1] < center[1]) {
            box.bounds[1][1] = center[1];
        } else {
            box.bounds[0][1] = center[1];
            index += 2;
        }

        if (p[2] < center[2]) {
            box.bounds[1][2] = center[2];
        } else {
            box.bounds[0][2] = center[2];
            index += 4;
        }
    }

    float hit_t;
    if (box.intersect_inside(ray, hit_t)) {
        if (ray.max_t > hit_t) {
            ray.max_t = hit_t;
        }

        //	std::cout << box.bounds[0] << " ";
        //	std::cout << p << std::endl;
    } else {
        //	std::cout << "min: " << box.bounds[0] << std::endl;
        //	std::cout << "max: " << box.bounds[1] << std::endl;
        //	std::cout << "p: " << p << std::endl;

        ray.max_t = ray.min_t;
        return false;
    }

    majorant_mu_t = nodes_[index].majorant_mu_t;

    return true;
}

Octree::Octree() : num_nodes_(0), nodes_(nullptr) {}

Octree::~Octree() {
    memory::free_aligned(nodes_);
}

Node* Octree::allocate_nodes(uint32_t num_nodes) {
    if (num_nodes != num_nodes_) {
        num_nodes_ = num_nodes;

        memory::free_aligned(nodes_);
        nodes_ = memory::allocate_aligned<Node>(num_nodes);
    }

    return nodes_;
}

void Octree::set_dimensions(int3 const& dimensions) {
    dimensions_       = dimensions;
    inv_2_dimensions_ = 2.f / float3(dimensions);
}

bool Octree::is_valid() const {
    return nullptr != nodes_;
}

bool Octree::intersect(math::Ray& ray, float& majorant_mu_t) const {
    //	{
    //		Box const box{{int3(0), dimensions_}};

    //		return intersect(ray, 0, box, majorant_mu_t);
    //	}

    float3 const p = ray.point(ray.min_t) + float3(1.f);

    if (math::any_negative(p)) {
        return false;
    }

    int3 const v = int3((0.5f * float3(dimensions_)) * p);

    if (math::any_greater_equal(v, dimensions_)) {
        return false;
    }

    Box box{{int3(0), dimensions_}};

    uint32_t index = 0;

    for (uint32_t l = 0;; ++l) {
        uint32_t const children = nodes_[index].children;

        if (0 == children) {
            break;
        }

        index = children;

        int3 const half = (box.bounds[1] - box.bounds[0]) / 2;

        int3 const center = box.bounds[0] + half;

        if (v[0] < center[0]) {
            box.bounds[1][0] = center[0];
        } else {
            box.bounds[0][0] = center[0];
            index += 1;
        }

        if (v[1] < center[1]) {
            box.bounds[1][1] = center[1];
        } else {
            box.bounds[0][1] = center[1];
            index += 2;
        }

        if (v[2] < center[2]) {
            box.bounds[1][2] = center[2];
        } else {
            box.bounds[0][2] = center[2];
            index += 4;
        }
    }

    float3 const min = inv_2_dimensions_ * float3(box.bounds[0]) - float3(1.f);
    float3 const max = inv_2_dimensions_ * float3(box.bounds[1]) - float3(1.f);

    math::AABB aabb(min, max);

    float hit_t;
    if (aabb.intersect_inside(ray, hit_t)) {
        if (ray.max_t > hit_t) {
            ray.max_t = hit_t;
        }
    } else {
        ray.max_t = ray.min_t;
        return false;
    }

    majorant_mu_t = nodes_[index].majorant_mu_t;

    return true;
}

bool Octree::intersect_f(math::Ray& ray, float& majorant_mu_t) const {
    //	{
    //		Box const box{{int3(0), dimensions_}};

    //		return intersect(ray, 0, box, majorant_mu_t);
    //	}

    //	return gridtree_.intersect(ray, majorant_mu_t);

    math::AABB box(float3(0.f), float3(1.f));

    float3 p = ray.point(ray.min_t);

    if (math::any_lesser_equal(p, 0.f) || math::any_greater_equal(p, 1.f)) {
        float hit_t;
        if (!box.intersect_p(ray, hit_t)) {
            return false;
        }

        ray.min_t = hit_t;
        p         = ray.point(ray.min_t);
    }

    uint32_t index = 0;

    for (uint32_t l = 0;; ++l) {
        uint32_t const children = nodes_[index].children;

        if (0 == children) {
            break;
        }

        index = children;

        float3 const half = box.halfsize();

        float3 const center = box.bounds[0] + half;

        if (p[0] < center[0]) {
            box.bounds[1][0] = center[0];
        } else {
            box.bounds[0][0] = center[0];
            index += 1;
        }

        if (p[1] < center[1]) {
            box.bounds[1][1] = center[1];
        } else {
            box.bounds[0][1] = center[1];
            index += 2;
        }

        if (p[2] < center[2]) {
            box.bounds[1][2] = center[2];
        } else {
            box.bounds[0][2] = center[2];
            index += 4;
        }
    }

    float hit_t;
    if (box.intersect_inside(ray, hit_t)) {
        if (ray.max_t > hit_t) {
            ray.max_t = hit_t;

            SOFT_ASSERT(ray.min_t <= hit_t);
        }

        //	std::cout << box.bounds[0] << " ";
        //	std::cout << p << std::endl;
    } else {
        //	std::cout << "min: " << box.bounds[0] << std::endl;
        //	std::cout << "max: " << box.bounds[1] << std::endl;
        //	std::cout << "p: " << p << std::endl;

        ray.max_t = ray.min_t;
        return false;
    }

    majorant_mu_t = nodes_[index].majorant_mu_t;

    return true;
}

bool Octree::intersect(math::Ray& ray, uint32_t node_id, Box const& box,
                       float& majorant_mu_t) const {
    float3 const min = inv_2_dimensions_ * float3(box.bounds[0]) - float3(1.f);
    float3 const max = inv_2_dimensions_ * float3(box.bounds[1]) - float3(1.f);

    math::AABB aabb(min, max);

    float hit_t;
    if (aabb.intersect_inside(ray, hit_t)) {
        if (ray.max_t > hit_t) {
            ray.max_t = hit_t;
        }

        auto const& node = nodes_[node_id];

        if (0 == node.children) {
            majorant_mu_t = node.majorant_mu_t;
            return true;
        }

        intersect_children(ray, node, box, majorant_mu_t);

        return true;
    }

    return false;
}

void Octree::intersect_children(math::Ray& ray, Node const& node, Box const& box,
                                float& majorant_mu_t) const {
    int3 const half = (box.bounds[1] - box.bounds[0]) / 2;

    int3 const center = box.bounds[0] + half;

    Box const sub0{{box.bounds[0], center}};
    intersect(ray, node.children + 0, sub0, majorant_mu_t);

    Box const sub1{{int3(center[0], box.bounds[0][1], box.bounds[0][2]),
                    int3(box.bounds[1][0], center[1], center[2])}};
    intersect(ray, node.children + 1, sub1, majorant_mu_t);

    Box const sub2{{int3(box.bounds[0][0], center[1], box.bounds[0][2]),
                    int3(center[0], box.bounds[1][1], center[2])}};
    intersect(ray, node.children + 2, sub2, majorant_mu_t);

    Box const sub3{{int3(center[0], center[1], box.bounds[0][2]),
                    int3(box.bounds[1][0], box.bounds[1][1], center[2])}};
    intersect(ray, node.children + 3, sub3, majorant_mu_t);

    Box const sub4{{int3(box.bounds[0][0], box.bounds[0][1], center[2]),
                    int3(center[0], center[1], box.bounds[1][2])}};
    intersect(ray, node.children + 4, sub4, majorant_mu_t);

    Box const sub5{{int3(center[0], box.bounds[0][1], center[2]),
                    int3(box.bounds[1][0], center[1], box.bounds[1][2])}};
    intersect(ray, node.children + 5, sub5, majorant_mu_t);

    Box const sub6{{int3(box.bounds[0][0], center[1], center[2]),
                    int3(center[0], box.bounds[1][1], box.bounds[1][2])}};
    intersect(ray, node.children + 6, sub6, majorant_mu_t);

    Box const sub7{{center, box.bounds[1]}};
    intersect(ray, node.children + 7, sub7, majorant_mu_t);
}

}  // namespace scene::material::volumetric
