#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_TREE_INL
#define SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_TREE_INL

#include "base/math/aabb.inl"
#include "base/math/ray.hpp"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "scene/bvh/scene_bvh_node.inl"
#include "scene/material/material.hpp"
#include "scene/scene_worker.hpp"
#include "scene/shape/node_stack.inl"
#include "scene/shape/shape_intersection.hpp"
#include "scene/shape/triangle/triangle_intersection.hpp"
#include "triangle_bvh_tree.hpp"

namespace scene::shape::triangle::bvh {

template <typename Data>
Tree<Data>::Tree() noexcept
    : num_nodes_(0), num_parts_(0), nodes_(nullptr), num_part_triangles_(nullptr) {}

template <typename Data>
Tree<Data>::~Tree() noexcept {
    delete[] num_part_triangles_;

    memory::free_aligned(nodes_);
}

template <typename Data>
scene::bvh::Node* Tree<Data>::allocate_nodes(uint32_t num_nodes) noexcept {
    if (num_nodes != num_nodes_) {
        num_nodes_ = num_nodes;

        memory::free_aligned(nodes_);
        nodes_ = memory::allocate_aligned<Node>(num_nodes);
    }

    return nodes_;
}

template <typename Data>
AABB Tree<Data>::aabb() const noexcept {
    if (nodes_) {
        return AABB(float3(nodes_[0].min()), float3(nodes_[0].max()));
    } else {
        return AABB::empty();
    }
}

template <typename Data>
uint32_t Tree<Data>::num_parts() const noexcept {
    return num_parts_;
}

template <typename Data>
uint32_t Tree<Data>::num_triangles() const noexcept {
    return data_.num_triangles();
}

template <typename Data>
uint32_t Tree<Data>::num_triangles(uint32_t part) const noexcept {
    return num_part_triangles_[part];
}

template <typename Data>
bool Tree<Data>::intersect(ray& ray, Node_stack& node_stack, Intersectioni& intersection) const
    noexcept {
    node_stack.push(0xFFFFFFFF);
    uint32_t n = 0;

    uint32_t index = 0xFFFFFFFF;

    float2 uv;

    while (0xFFFFFFFF != n) {
        auto const& node = nodes_[n];

        if (node.intersect_p(ray)) {
            if (0 == node.num_primitives()) {
                if (0 == ray.signs[node.axis()]) {
                    node_stack.push(node.next());
                    ++n;
                } else {
                    node_stack.push(n + 1);
                    n = node.next();
                }

                continue;
            }

            for (uint32_t i = node.indices_start(), len = node.indices_end(); i < len; ++i) {
                if (data_.intersect(i, ray, uv)) {
                    index = i;
                }
            }
        }

        n = node_stack.pop();
    }

    if (index != 0xFFFFFFFF) {
        intersection.uv    = uv;
        intersection.index = index;
        return true;
    }

    return false;
}

template <typename Data>
bool Tree<Data>::intersect(ray& ray, Node_stack& node_stack, Intersection& intersection) const
    noexcept {
    node_stack.push(0xFFFFFFFF);
    uint32_t n = 0;

    uint32_t index = 0xFFFFFFFF;

    Vector const ray_origin        = simd::load_float4(ray.origin.v);
    Vector const ray_direction     = simd::load_float4(ray.direction.v);
    Vector const ray_inv_direction = simd::load_float4(ray.inv_direction.v);
    Vector const ray_min_t         = simd::load_float(&ray.min_t);
    Vector       ray_max_t         = simd::load_float(&ray.max_t);
    Vector       u;
    Vector       v;

    while (0xFFFFFFFF != n) {
        auto const& node = nodes_[n];

        if (node.intersect_p(ray_origin, ray_inv_direction, ray_min_t, ray_max_t)) {
            if (0 == node.num_primitives()) {
                if (0 == ray.signs[node.axis()]) {
                    node_stack.push(node.next());
                    ++n;
                } else {
                    node_stack.push(n + 1);
                    n = node.next();
                }

                continue;
            }

            for (uint32_t i = node.indices_start(), len = node.indices_end(); i < len; ++i) {
                if (data_.intersect(ray_origin, ray_direction, ray_min_t, ray_max_t, i, u, v)) {
                    index = i;
                }
            }
        }

        n = node_stack.pop();
    }

    if (index != 0xFFFFFFFF) {
        _mm_store_ss(&ray.max_t, ray_max_t);
        // ray.max_t = _mm_cvtss_f32(ray_max_t);
        intersection.u     = math::splat_x(u);
        intersection.v     = math::splat_x(v);
        intersection.index = index;
        return true;
    }

    return false;
}

template <typename Data>
bool Tree<Data>::intersect(ray& ray, Node_stack& node_stack) const noexcept {
    node_stack.push(0xFFFFFFFF);
    uint32_t n = 0;

    uint32_t index = 0xFFFFFFFF;

    Vector const ray_origin        = simd::load_float4(ray.origin.v);
    Vector const ray_direction     = simd::load_float4(ray.direction.v);
    Vector const ray_inv_direction = simd::load_float4(ray.inv_direction.v);
    Vector const ray_min_t         = simd::load_float(&ray.min_t);
    Vector       ray_max_t         = simd::load_float(&ray.max_t);

    while (0xFFFFFFFF != n) {
        auto const& node = nodes_[n];

        if (node.intersect_p(ray_origin, ray_inv_direction, ray_min_t, ray_max_t)) {
            if (0 == node.num_primitives()) {
                if (0 == ray.signs[node.axis()]) {
                    node_stack.push(node.next());
                    ++n;
                } else {
                    node_stack.push(n + 1);
                    n = node.next();
                }

                continue;
            }

            for (uint32_t i = node.indices_start(), len = node.indices_end(); i < len; ++i) {
                if (data_.intersect(ray_origin, ray_direction, ray_min_t, ray_max_t, i)) {
                    index = i;
                }
            }
        }

        n = node_stack.pop();
    }

    if (index != 0xFFFFFFFF) {
        _mm_store_ss(&ray.max_t, ray_max_t);
        return true;
    }

    return false;
}

template <typename Data>
bool Tree<Data>::intersect(FVector ray_origin, FVector ray_direction, FVector ray_inv_direction,
                           FVector ray_min_t, Vector& ray_max_t, uint32_t ray_signs[4],
                           Node_stack& node_stack, Intersection& intersection) const noexcept {
    node_stack.push(0xFFFFFFFF);
    uint32_t n = 0;

    uint32_t index = 0xFFFFFFFF;

    Vector u;
    Vector v;

    while (0xFFFFFFFF != n) {
        auto const& node = nodes_[n];

        if (node.intersect_p(ray_origin, ray_inv_direction, ray_min_t, ray_max_t)) {
            if (0 == node.num_primitives()) {
                if (0 == ray_signs[node.axis()]) {
                    node_stack.push(node.next());
                    ++n;
                } else {
                    node_stack.push(n + 1);
                    n = node.next();
                }

                continue;
            }

            for (uint32_t i = node.indices_start(), len = node.indices_end(); i < len; ++i) {
                if (data_.intersect(ray_origin, ray_direction, ray_min_t, ray_max_t, i, u, v)) {
                    index = i;
                }
            }
        }

        n = node_stack.pop();
    }

    if (index != 0xFFFFFFFF) {
        intersection.u     = math::splat_x(u);
        intersection.v     = math::splat_x(v);
        intersection.index = index;
        return true;
    }

    return false;
}

template <typename Data>
bool Tree<Data>::intersect(FVector ray_origin, FVector ray_direction, FVector ray_inv_direction,
                           FVector ray_min_t, Vector& ray_max_t, uint32_t ray_signs[4],
                           Node_stack& node_stack) const noexcept {
    node_stack.push(0xFFFFFFFF);
    uint32_t n = 0;

    uint32_t index = 0xFFFFFFFF;

    while (0xFFFFFFFF != n) {
        auto const& node = nodes_[n];

        if (node.intersect_p(ray_origin, ray_inv_direction, ray_min_t, ray_max_t)) {
            if (0 == node.num_primitives()) {
                if (0 == ray_signs[node.axis()]) {
                    node_stack.push(node.next());
                    ++n;
                } else {
                    node_stack.push(n + 1);
                    n = node.next();
                }

                continue;
            }

            for (uint32_t i = node.indices_start(), len = node.indices_end(); i < len; ++i) {
                if (data_.intersect(ray_origin, ray_direction, ray_min_t, ray_max_t, i)) {
                    index = i;
                }
            }
        }

        n = node_stack.pop();
    }

    if (index != 0xFFFFFFFF) {
        return true;
    }

    return false;
}

template <typename Data>
bool Tree<Data>::intersect_p(ray const& ray, Node_stack& node_stack) const noexcept {
    node_stack.push(0xFFFFFFFF);
    uint32_t n = 0;

    Vector const ray_origin        = simd::load_float4(ray.origin.v);
    Vector const ray_direction     = simd::load_float4(ray.direction.v);
    Vector const ray_inv_direction = simd::load_float4(ray.inv_direction.v);
    Vector const ray_min_t         = simd::load_float(&ray.min_t);
    Vector const ray_max_t         = simd::load_float(&ray.max_t);

    while (0xFFFFFFFF != n) {
        auto const& node = nodes_[n];

        if (node.intersect_p(ray_origin, ray_inv_direction, ray_min_t, ray_max_t)) {
            if (0 == node.num_primitives()) {
                if (0 == ray.signs[node.axis()]) {
                    node_stack.push(node.next());
                    ++n;
                } else {
                    node_stack.push(n + 1);
                    n = node.next();
                }

                continue;
            }

            for (uint32_t i = node.indices_start(), len = node.indices_end(); i < len; ++i) {
                if (data_.intersect_p(ray_origin, ray_direction, ray_min_t, ray_max_t, i)) {
                    return true;
                }
            }
        }

        n = node_stack.pop();
    }

    return false;
}

template <typename Data>
bool Tree<Data>::intersect_p(FVector ray_origin, FVector ray_direction, FVector ray_inv_direction,
                             FVector ray_min_t, FVector ray_max_t, uint32_t ray_signs[4],
                             Node_stack& node_stack) const noexcept {
    node_stack.push(0xFFFFFFFF);
    uint32_t n = 0;

    while (0xFFFFFFFF != n) {
        auto const& node = nodes_[n];

        if (node.intersect_p(ray_origin, ray_inv_direction, ray_min_t, ray_max_t)) {
            if (0 == node.num_primitives()) {
                if (0 == ray_signs[node.axis()]) {
                    node_stack.push(node.next());
                    ++n;
                } else {
                    node_stack.push(n + 1);
                    n = node.next();
                }

                continue;
            }

            for (uint32_t i = node.indices_start(), len = node.indices_end(); i < len; ++i) {
                if (data_.intersect_p(ray_origin, ray_direction, ray_min_t, ray_max_t, i)) {
                    return true;
                }
            }
        }

        n = node_stack.pop();
    }

    return false;
}

template <typename Data>
float Tree<Data>::opacity(ray& ray, uint64_t time, Materials materials, Filter filter,
                          Worker const& worker) const noexcept {
    auto& node_stack = worker.node_stack();
    //	node_stack.clear();
    //	node_stack.push(0);
    node_stack.push(0xFFFFFFFF);
    uint32_t n = 0;

    float opacity = 0.f;

    Vector const ray_origin        = simd::load_float4(ray.origin.v);
    Vector const ray_direction     = simd::load_float4(ray.direction.v);
    Vector const ray_inv_direction = simd::load_float4(ray.inv_direction.v);
    Vector const ray_min_t         = simd::load_float(&ray.min_t);
    Vector       ray_max_t         = simd::load_float(&ray.max_t);
    Vector const max_t             = ray_max_t;

    Vector u;
    Vector v;

    //	while (!node_stack.empty()) {
    while (0xFFFFFFFF != n) {
        auto& node = nodes_[n];

        if (node.intersect_p(ray_origin, ray_inv_direction, ray_min_t, ray_max_t)) {
            if (0 == node.num_primitives()) {
                if (0 == ray.signs[node.axis()]) {
                    node_stack.push(node.next());
                    ++n;
                } else {
                    node_stack.push(n + 1);
                    n = node.next();
                }

                continue;
            }

            for (uint32_t i = node.indices_start(), len = node.indices_end(); i < len; ++i) {
                if (data_.intersect(ray_origin, ray_direction, ray_min_t, ray_max_t, i, u, v)) {
                    u         = math::splat_x(u);
                    v         = math::splat_x(v);
                    float2 uv = data_.interpolate_uv(u, v, i);

                    auto const material = materials[data_.material_index(i)];

                    opacity += (1.f - opacity) * material->opacity(uv, time, filter, worker);
                    if (opacity >= 1.f) {
                        return 1.f;
                    }

                    // ray_max_t has changed if intersect() returns true!
                    ray_max_t = max_t;
                }
            }
        }

        n = node_stack.pop();
    }

    return opacity;
}

template <typename Data>
bool Tree<Data>::absorption(ray& ray, uint64_t time, Materials materials, Filter filter,
                            Worker const& worker, float3& ta) const noexcept {
    auto& node_stack = worker.node_stack();
    //	node_stack.clear();
    //	node_stack.push(0);
    node_stack.push(0xFFFFFFFF);
    uint32_t n = 0;

    float3 absorption(1.f);

    Vector const ray_origin        = simd::load_float4(ray.origin.v);
    Vector const ray_direction     = simd::load_float4(ray.direction.v);
    Vector const ray_inv_direction = simd::load_float4(ray.inv_direction.v);
    Vector const ray_min_t         = simd::load_float(&ray.min_t);
    Vector       ray_max_t         = simd::load_float(&ray.max_t);
    Vector const max_t             = ray_max_t;

    Vector u;
    Vector v;

    //	while (!node_stack.empty()) {
    while (0xFFFFFFFF != n) {
        auto& node = nodes_[n];

        if (node.intersect_p(ray_origin, ray_inv_direction, ray_min_t, ray_max_t)) {
            if (0 == node.num_primitives()) {
                if (0 == ray.signs[node.axis()]) {
                    node_stack.push(node.next());
                    ++n;
                } else {
                    node_stack.push(n + 1);
                    n = node.next();
                }

                continue;
            }

            for (uint32_t i = node.indices_start(), len = node.indices_end(); i < len; ++i) {
                if (data_.intersect(ray_origin, ray_direction, ray_min_t, ray_max_t, i, u, v)) {
                    u = math::splat_x(u);
                    v = math::splat_x(v);

                    float2 uv = data_.interpolate_uv(u, v, i);

                    float3 const normal = data_.normal(i);

                    auto const material = materials[data_.material_index(i)];

                    float3 const tta = material->thin_absorption(ray.direction, normal, uv, time,
                                                                 filter, worker);
                    absorption *= tta;
                    if (all_equal_zero(absorption)) {
                        ta = float3(0.f);
                        return false;
                    }

                    // ray_max_t has changed if intersect() returns true!
                    ray_max_t = max_t;
                }
            }
        }

        n = node_stack.pop();
    }

    ta = absorption;
    return true;
}

template <typename Data>
float3 Tree<Data>::interpolate_p(float2 uv, uint32_t index) const noexcept {
    return data_.interpolate_p(uv, index);
}

template <typename Data>
Simd3f Tree<Data>::interpolate_p(Simd3f const& u, Simd3f const& v, uint32_t index) const noexcept {
    return data_.interpolate_p(u, v, index);
}

template <typename Data>
void Tree<Data>::interpolate_triangle_data(uint32_t index, float2 uv, float3& n, float3& t,
                                           float2& tc) const noexcept {
    data_.interpolate_data(index, uv, n, t, tc);
}

template <typename Data>
void Tree<Data>::interpolate_triangle_data(Simd3f const& u, Simd3f const& v, uint32_t index,
                                           Simd3f& n, Simd3f& t, float2& tc) const noexcept {
    data_.interpolate_data(u, v, index, n, t, tc);
}

template <typename Data>
Vector Tree<Data>::interpolate_shading_normal(FVector u, FVector v, uint32_t index) const noexcept {
    return data_.interpolate_shading_normal(u, v, index);
}

template <typename Data>
float2 Tree<Data>::interpolate_triangle_uv(uint32_t index, float2 uv) const noexcept {
    return data_.interpolate_uv(index, uv);
}

template <typename Data>
float2 Tree<Data>::interpolate_triangle_uv(Simd3f const& u, Simd3f const& v, uint32_t index) const
    noexcept {
    return data_.interpolate_uv(u, v, index);
}

template <typename Data>
float Tree<Data>::triangle_bitangent_sign(uint32_t index) const noexcept {
    return data_.bitangent_sign(index);
}

template <typename Data>
uint32_t Tree<Data>::triangle_material_index(uint32_t index) const noexcept {
    return data_.material_index(index);
}

template <typename Data>
float3 Tree<Data>::triangle_normal(uint32_t index) const noexcept {
    return data_.normal(index);
}

template <typename Data>
Simd3f Tree<Data>::triangle_normal_v(uint32_t index) const noexcept {
    return data_.normal_v(index);
}

template <typename Data>
float Tree<Data>::triangle_area(uint32_t index) const noexcept {
    return data_.area(index);
}

template <typename Data>
float Tree<Data>::triangle_area(uint32_t index, float3 const& scale) const noexcept {
    return data_.area(index, scale);
}

// template <typename Data>
// void Tree<Data>::sample(uint32_t index, float2 r2, float3& p, float3& n, float2& tc) const
//    noexcept {
//    data_.sample(index, r2, p, n, tc);
//}

template <typename Data>
void Tree<Data>::sample(uint32_t index, float2 r2, float3& p, float2& tc) const noexcept {
    data_.sample(index, r2, p, tc);
}

// template <typename Data>
// void Tree<Data>::sample(uint32_t index, float2 r2, float3& p) const noexcept {
//    data_.sample(index, r2, p);
//}

template <typename Data>
void Tree<Data>::allocate_parts(uint32_t num_parts) noexcept {
    if (num_parts != num_parts_) {
        num_parts_ = num_parts;
        delete[] num_part_triangles_;
        num_part_triangles_ = new uint32_t[num_parts];
    }

    for (uint32_t i = 0; i < num_parts; ++i) {
        num_part_triangles_[i] = 0;
    }
}

template <typename Data>
void Tree<Data>::allocate_triangles(uint32_t             num_triangles,
                                    Vertex_stream const& vertices) noexcept {
    data_.allocate_triangles(num_triangles, vertices);
}

template <typename Data>
void Tree<Data>::add_triangle(uint32_t a, uint32_t b, uint32_t c, uint32_t material_index,
                              Vertex_stream const& vertices, uint32_t current_triangle) noexcept {
    ++num_part_triangles_[material_index];

    data_.add_triangle(a, b, c, material_index, vertices, current_triangle);
}

template <typename Data>
size_t Tree<Data>::num_bytes() const noexcept {
    return sizeof(*this) + num_nodes_ * sizeof(Node) + num_parts_ * sizeof(uint32_t) +
           data_.num_bytes();
}

}  // namespace scene::shape::triangle::bvh

#endif
