#include "triangle_bvh_tree.hpp"
#include "base/math/aabb.inl"
#include "base/math/ray.inl"
#include "base/math/vector3.inl"
#include "scene/bvh/scene_bvh_node.inl"
#include "scene/material/material.hpp"
#include "scene/scene.inl"
#include "scene/scene_worker.inl"
#include "scene/shape/node_stack.inl"
#include "scene/shape/shape_intersection.hpp"
#include "triangle_bvh_indexed_data.inl"

namespace scene::shape::triangle::bvh {

Tree::Tree() : num_nodes_(0), num_parts_(0), nodes_(nullptr) {}

Tree::~Tree() {
    delete[] nodes_;
}

scene::bvh::Node* Tree::allocate_nodes(uint32_t num_nodes) {
    if (num_nodes != num_nodes_) {
        num_nodes_ = num_nodes;

        delete[] nodes_;
        nodes_ = new Node[num_nodes];
    }

    return nodes_;
}

AABB Tree::aabb() const {
    if (nodes_) {
        return AABB(float3(nodes_[0].min()), float3(nodes_[0].max()));
    } else {
        return Empty_AABB;
    }
}

bool Tree::intersect(Simdf_p ray_origin, Simdf_p ray_direction, scalar_p ray_min_t,
                     scalar& ray_max_t, Node_stack& nodes, Intersection& isec) const {
    Simdf const ray_inv_direction = reciprocal3(ray_direction);

    alignas(16) uint32_t ray_signs[4];
    sign(ray_inv_direction, ray_signs);

    nodes.push(0xFFFFFFFF);
    uint32_t n = 0;

    uint32_t index = 0xFFFFFFFF;

    SimdVec const o = {ray_origin.splat_x(), ray_origin.splat_y(), ray_origin.splat_z()};
    SimdVec const d = {ray_direction.splat_x(), ray_direction.splat_y(), ray_direction.splat_z()};

    Simdf const mintolo(ray_min_t);
    Simdf maxtolo(ray_max_t);

    Simdf u;
    Simdf v;

    bool hit = false;

    while (0xFFFFFFFF != n) {
        auto const& node = nodes_[n];

        if (node.intersect_p(ray_origin, ray_inv_direction, ray_min_t, ray_max_t)) {
            if (0 == node.num_indices()) {
                uint32_t const a = node.children();
                uint32_t const b = a + 1;

                if (0 == ray_signs[node.axis()]) {
                    nodes.push(b);
                    n = a;
                } else {
                    nodes.push(a);
                    n = b;
                }

                continue;
            }

            if (data_.intersect(o, d, mintolo, maxtolo, node.indices_start(), node.indices_end(), u,
                                v, index)) {
                ray_max_t = scalar(maxtolo.v);

                hit = true;
            }
        }

        n = nodes.pop();
    }

    if (hit) {
    isec.u     = u;
    isec.v     = v;
    isec.index = index;
    return true;
    }

    return false;
}

bool Tree::intersect_p(Simdf_p ray_origin, Simdf_p ray_direction, scalar_p ray_min_t,
                       scalar_p ray_max_t, Node_stack& nodes) const {
    Simdf const ray_inv_direction = reciprocal3(ray_direction);

    alignas(16) uint32_t ray_signs[4];
    sign(ray_inv_direction, ray_signs);

    SimdVec const o = {ray_origin.splat_x(), ray_origin.splat_y(), ray_origin.splat_z()};
    SimdVec const d = {ray_direction.splat_x(), ray_direction.splat_y(), ray_direction.splat_z()};

    Simdf const mintolo(ray_min_t);
    Simdf const maxtolo(ray_max_t);

    nodes.push(0xFFFFFFFF);
    uint32_t n = 0;

    while (0xFFFFFFFF != n) {
        auto const& node = nodes_[n];

        if (node.intersect_p(ray_origin, ray_inv_direction, ray_min_t, ray_max_t)) {
            if (0 == node.num_indices()) {
                uint32_t const a = node.children();
                uint32_t const b = a + 1;

                if (0 == ray_signs[node.axis()]) {
                    nodes.push(b);
                    n = a;
                } else {
                    nodes.push(a);
                    n = b;
                }

                continue;
            }

            if (data_.intersect_p(o, d, mintolo, maxtolo, node.indices_start(),
                                  node.indices_end())) {
                return true;
            }
        }

        n = nodes.pop();
    }

    return false;
}

bool Tree::visibility(Simdf_p ray_origin, Simdf_p ray_direction, scalar_p ray_min_t,
                      scalar_p ray_max_t, uint32_t entity, Filter filter, Worker& worker,
                      float3& vis) const {
    Simdf const ray_inv_direction = reciprocal3(ray_direction);

    alignas(16) uint32_t ray_signs[4];
    sign(ray_inv_direction, ray_signs);

    auto& nodes = worker.node_stack();

    nodes.push(0xFFFFFFFF);
    uint32_t n = 0;

    SimdVec const o = {ray_origin.splat_x(), ray_origin.splat_y(), ray_origin.splat_z()};
    SimdVec const d = {ray_direction.splat_x(), ray_direction.splat_y(), ray_direction.splat_z()};

    Simdf const mintolo(ray_min_t);
    Simdf const maxtolo(ray_max_t);

    float3 const ray_dir(ray_direction);

    float3 local_vis(1.f);

    while (0xFFFFFFFF != n) {
        auto const& node = nodes_[n];

        if (node.intersect_p(ray_origin, ray_inv_direction, ray_min_t, ray_max_t)) {
            if (0 == node.num_indices()) {
                uint32_t const a = node.children();
                uint32_t const b = a + 1;

                if (0 == ray_signs[node.axis()]) {
                    nodes.push(b);
                    n = a;
                } else {
                    nodes.push(a);
                    n = b;
                }

                continue;
            }

            float3 tv;
            if (!data_.visibility(o, d, mintolo, maxtolo, node.indices_start(), node.indices_end(),
                                  ray_dir, entity, filter, worker, tv)) {
                return false;
            }

            local_vis *= tv;
        }

        n = nodes.pop();
    }

    vis = local_vis;
    return true;
}

}  // namespace scene::shape::triangle::bvh
