#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_TREE_INL
#define SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_TREE_INL

#include "base/math/aabb.inl"
#include "base/math/ray.hpp"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "scene/bvh/scene_bvh_node.inl"
#include "scene/scene_worker.hpp"
#include "scene/shape/node_stack.inl"
#include "scene/shape/triangle/triangle_intersection.hpp"
#include "triangle_bvh_tree.hpp"

namespace scene::shape::triangle::bvh {

template <typename Data>
Tree<Data>::Tree() : num_nodes_(0), nodes_(nullptr), num_parts_(0), num_part_triangles_(nullptr) {}

template <typename Data>
Tree<Data>::~Tree() {
    delete[] num_part_triangles_;

    memory::free_aligned(nodes_);
}

template <typename Data>
scene::bvh::Node* Tree<Data>::allocate_nodes(uint32_t num_nodes) {
    if (num_nodes != num_nodes_) {
        num_nodes_ = num_nodes;

        memory::free_aligned(nodes_);
        nodes_ = memory::allocate_aligned<Node>(num_nodes);
    }

    return nodes_;
}

template <typename Data>
math::AABB Tree<Data>::aabb() const {
    if (nodes_) {
        return math::AABB(float3(nodes_[0].min()), float3(nodes_[0].max()));
    } else {
        return math::AABB::empty();
    }
}

template <typename Data>
uint32_t Tree<Data>::num_parts() const {
    return num_parts_;
}

template <typename Data>
uint32_t Tree<Data>::num_triangles() const {
    return data_.num_triangles();
}

template <typename Data>
uint32_t Tree<Data>::num_triangles(uint32_t part) const {
    return num_part_triangles_[part];
}

template <typename Data>
uint32_t Tree<Data>::current_triangle() const {
    return data_.current_triangle();
}

template <typename Data>
bool Tree<Data>::intersect(math::Ray& ray, Node_stack& node_stack,
                           Intersection& intersection) const {
    node_stack.push(0xFFFFFFFF);
    uint32_t n = 0;

    uint32_t index = 0xFFFFFFFF;

    const Vector ray_origin        = simd::load_float4(ray.origin.v);
    const Vector ray_direction     = simd::load_float4(ray.direction.v);
    const Vector ray_inv_direction = simd::load_float4(ray.inv_direction.v);
    const Vector ray_min_t         = simd::load_float(&ray.min_t);
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
bool Tree<Data>::intersect(math::Ray& ray, Node_stack& node_stack) const {
    node_stack.push(0xFFFFFFFF);
    uint32_t n = 0;

    uint32_t index = 0xFFFFFFFF;

    const Vector ray_origin        = simd::load_float4(ray.origin.v);
    const Vector ray_direction     = simd::load_float4(ray.direction.v);
    const Vector ray_inv_direction = simd::load_float4(ray.inv_direction.v);
    const Vector ray_min_t         = simd::load_float(&ray.min_t);
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
                           Node_stack& node_stack, Intersection& intersection) const {
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
                           Node_stack& node_stack) const {
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
bool Tree<Data>::intersect_p(math::Ray const& ray, Node_stack& node_stack) const {
    node_stack.push(0xFFFFFFFF);
    uint32_t n = 0;

    const Vector ray_origin        = simd::load_float4(ray.origin.v);
    const Vector ray_direction     = simd::load_float4(ray.direction.v);
    const Vector ray_inv_direction = simd::load_float4(ray.inv_direction.v);
    const Vector ray_min_t         = simd::load_float(&ray.min_t);
    const Vector ray_max_t         = simd::load_float(&ray.max_t);

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
                             Node_stack& node_stack) const {
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
float Tree<Data>::opacity(math::Ray& ray, float time, Materials const& materials,
                          material::Sampler_settings::Filter filter, Worker const& worker) const {
    auto& node_stack = worker.node_stack();
    //	node_stack.clear();
    //	node_stack.push(0);
    node_stack.push(0xFFFFFFFF);
    uint32_t n = 0;

    float opacity = 0.f;

    const Vector ray_origin        = simd::load_float4(ray.origin.v);
    const Vector ray_direction     = simd::load_float4(ray.direction.v);
    const Vector ray_inv_direction = simd::load_float4(ray.inv_direction.v);
    const Vector ray_min_t         = simd::load_float(&ray.min_t);
    Vector       ray_max_t         = simd::load_float(&ray.max_t);
    const Vector max_t             = ray_max_t;

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
float3 Tree<Data>::absorption(math::Ray& ray, float time, Materials const& materials,
                              material::Sampler_settings::Filter filter,
                              Worker const&                      worker) const {
    auto& node_stack = worker.node_stack();
    //	node_stack.clear();
    //	node_stack.push(0);
    node_stack.push(0xFFFFFFFF);
    uint32_t n = 0;

    float3 absorption(0.f);

    const Vector ray_origin        = simd::load_float4(ray.origin.v);
    const Vector ray_direction     = simd::load_float4(ray.direction.v);
    const Vector ray_inv_direction = simd::load_float4(ray.inv_direction.v);
    const Vector ray_min_t         = simd::load_float(&ray.min_t);
    Vector       ray_max_t         = simd::load_float(&ray.max_t);
    const Vector max_t             = ray_max_t;

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

                    float3 const normal = data_.normal(i);

                    auto const material = materials[data_.material_index(i)];

                    float3 const ta = material->thin_absorption(ray.direction, normal, uv, time,
                                                                filter, worker);
                    absorption += (1.f - absorption) * ta;
                    if (math::all_greater_equal(absorption, 1.f)) {
                        return float3(1.f);
                    }

                    // ray_max_t has changed if intersect() returns true!
                    ray_max_t = max_t;
                }
            }
        }

        n = node_stack.pop();
    }

    return absorption;
}

template <typename Data>
void Tree<Data>::interpolate_triangle_data(uint32_t index, float2 uv, float3& n, float3& t,
                                           float2& tc) const {
    data_.interpolate_data(index, uv, n, t, tc);
}

template <typename Data>
void Tree<Data>::interpolate_triangle_data(FVector u, FVector v, uint32_t index, float3& n,
                                           float3& t, float2& tc) const {
    data_.interpolate_data(u, v, index, n, t, tc);
}

template <typename Data>
void Tree<Data>::interpolate_triangle_data(FVector u, FVector v, uint32_t index, Vector& n,
                                           Vector& t, float2& tc) const {
    data_.interpolate_data(u, v, index, n, t, tc);
}

template <typename Data>
float2 Tree<Data>::interpolate_triangle_uv(uint32_t index, float2 uv) const {
    return data_.interpolate_uv(index, uv);
}

template <typename Data>
float2 Tree<Data>::interpolate_triangle_uv(FVector u, FVector v, uint32_t index) const {
    return data_.interpolate_uv(u, v, index);
}

template <typename Data>
float Tree<Data>::triangle_bitangent_sign(uint32_t index) const {
    return data_.bitangent_sign(index);
}

template <typename Data>
uint32_t Tree<Data>::triangle_material_index(uint32_t index) const {
    return data_.material_index(index);
}

template <typename Data>
float3 Tree<Data>::triangle_normal(uint32_t index) const {
    return data_.normal(index);
}

template <typename Data>
Vector Tree<Data>::triangle_normal_v(uint32_t index) const {
    return data_.normal_v(index);
}

template <typename Data>
float Tree<Data>::triangle_area(uint32_t index) const {
    return data_.area(index);
}

template <typename Data>
float Tree<Data>::triangle_area(uint32_t index, f_float3 scale) const {
    return data_.area(index, scale);
}

template <typename Data>
void Tree<Data>::sample(uint32_t index, float2 r2, float3& p, float3& n, float2& tc) const {
    data_.sample(index, r2, p, n, tc);
}

template <typename Data>
void Tree<Data>::sample(uint32_t index, float2 r2, float3& p, float2& tc) const {
    data_.sample(index, r2, p, tc);
}

template <typename Data>
void Tree<Data>::sample(uint32_t index, float2 r2, float3& p) const {
    data_.sample(index, r2, p);
}

template <typename Data>
void Tree<Data>::allocate_parts(uint32_t num_parts) {
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
void Tree<Data>::allocate_triangles(uint32_t num_triangles, const std::vector<Vertex>& vertices) {
    data_.allocate_triangles(num_triangles, vertices);
}

template <typename Data>
void Tree<Data>::add_triangle(uint32_t a, uint32_t b, uint32_t c, uint32_t material_index,
                              const std::vector<Vertex>& vertices) {
    ++num_part_triangles_[material_index];

    data_.add_triangle(a, b, c, material_index, vertices);
}

template <typename Data>
size_t Tree<Data>::num_bytes() const {
    return sizeof(*this) + num_nodes_ * sizeof(Node) + num_parts_ * sizeof(uint32_t) +
           data_.num_bytes();
}

}  // namespace scene::shape::triangle::bvh

#endif
