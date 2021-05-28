#include "triangle_bvh_tree.hpp"
#include "base/math/aabb.inl"
#include "base/math/ray.inl"
#include "base/math/vector3.inl"
#include "scene/bvh/scene_bvh_node.inl"
#include "scene/material/material.hpp"
#include "scene/material/material.inl"
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

    SimdVec o = {ray_origin.splat_x(), ray_origin.splat_y(), ray_origin.splat_z()};
    SimdVec d = {ray_direction.splat_x(), ray_direction.splat_y(), ray_direction.splat_z()};

    Simdf mintolo(ray_min_t);
    Simdf maxtolo(ray_max_t);

    Simdf u;
    Simdf v;

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

             if (data_.intersect(o, d, mintolo, maxtolo,
                                    node.indices_start(), node.indices_end(), u, v, index)) {
                 ray_max_t = scalar(maxtolo.v);
             }


        }

        n = nodes.pop();
    }

    isec.u     = u;
    isec.v     = v;
    isec.index = index;

    return index != 0xFFFFFFFF;
}

bool Tree::intersect_p(Simdf_p ray_origin, Simdf_p ray_direction, scalar_p ray_min_t,
                       scalar_p ray_max_t, Node_stack& nodes) const {
    Simdf const ray_inv_direction = reciprocal3(ray_direction);

    alignas(16) uint32_t ray_signs[4];
    sign(ray_inv_direction, ray_signs);


    SimdVec o = {ray_origin.splat_x(), ray_origin.splat_y(), ray_origin.splat_z()};
    SimdVec d = {ray_direction.splat_x(), ray_direction.splat_y(), ray_direction.splat_z()};

    Simdf mintolo(ray_min_t);
    Simdf maxtolo(ray_max_t);

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

            if (data_.intersect_p(o, d, mintolo, maxtolo,
                                node.indices_start(), node.indices_end())) {
                return true;
            }
        }

        n = nodes.pop();
    }

    return false;
}

bool Tree::visibility(ray& ray, uint32_t entity, Filter filter, Worker& worker, float3& vis) const {
    auto& nodes = worker.node_stack();

    nodes.push(0xFFFFFFFF);
    uint32_t n = 0;

    float3 local_vis(1.f);

    Simdf const  ray_origin(ray.origin.v);
    Simdf const  ray_direction(ray.direction.v);
    Simdf const  ray_inv_direction(ray.inv_direction.v);
    scalar const ray_min_t(ray.min_t());
    scalar       ray_max_t(ray.max_t());
    scalar const max_t = ray_max_t;

    alignas(16) uint32_t ray_signs[4];
    sign(ray_inv_direction, ray_signs);

    scalar u;
    scalar v;

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

            for (uint32_t i = node.indices_start(), len = node.indices_end(); i < len; ++i) {
                if (data_.intersect(ray_origin, ray_direction, ray_min_t, ray_max_t, i, u, v)) {
                    float2 const uv = data_.interpolate_uv(Simdf(u), Simdf(v), i);

                    float3 const normal = float3(data_.normal(i));

                    auto const material = worker.scene().prop_material(entity, data_.part(i));

                    float3 tv;
                    if (!material->visibility(ray.direction, normal, uv, filter, worker, tv)) {
                        return false;
                    }

                    local_vis *= tv;

                    // ray_max_t has changed if intersect() returns true!
                    ray_max_t = max_t;
                }
            }
        }

        n = nodes.pop();
    }

    vis = local_vis;
    return true;
}

}  // namespace scene::shape::triangle::bvh
