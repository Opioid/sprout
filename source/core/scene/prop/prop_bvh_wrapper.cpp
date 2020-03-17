#include "prop_bvh_wrapper.hpp"
#include "prop.hpp"
#include "prop_intersection.hpp"
#include "scene/bvh/scene_bvh_node.inl"
#include "scene/bvh/scene_bvh_tree.inl"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.inl"
#include "scene/shape/node_stack.inl"

namespace scene::prop {

bvh::Tree& BVH_wrapper::tree() {
    return tree_;
}

void BVH_wrapper::set_props(std::vector<uint32_t> const& infinite_props,
                            std::vector<Prop> const&     props) {
    num_infinite_props_ = uint32_t(infinite_props.size());

    infinite_props_ = infinite_props.data();

    props_ = props.data();
}

AABB const& BVH_wrapper::aabb() const {
    return tree_.aabb_;
}

bool BVH_wrapper::intersect(Ray& ray, Worker& worker, Intersection& intersection) const {
    auto& node_stack = worker.node_stack();

    bool hit = false;

    uint32_t prop = 0xFFFFFFFF;

    node_stack.clear();
    if (0 != tree_.num_nodes_) {
        node_stack.push(0);
    }

    uint32_t n = 0;

    Simd3f const ray_origin(ray.origin.v);
    Simd3f const ray_inv_direction(ray.inv_direction.v);
    scalar const ray_min_t(ray.min_t());
    scalar       ray_max_t(ray.max_t());

    alignas(16) uint32_t ray_signs[4];
    sign(ray_inv_direction, ray_signs);

    bvh::Node* nodes = tree_.nodes_;

    Prop const* props = props_;

    uint32_t const* finite_props = tree_.indices_;

    while (!node_stack.empty()) {
        auto const& node = nodes[n];

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
                uint32_t const p = finite_props[i];
                if (props[p].intersect(p, ray, worker, intersection.geo)) {
                    prop      = p;
                    hit       = true;
                    ray_max_t = scalar(ray.max_t());
                }
            }
        }

        n = node_stack.pop();
    }

    uint32_t const* infinite_props = infinite_props_;

    for (uint32_t i = 0, len = num_infinite_props_; i < len; ++i) {
        uint32_t const p = infinite_props[i];
        if (props[p].intersect(p, ray, worker, intersection.geo)) {
            prop = p;
            hit  = true;
        }
    }

    intersection.prop       = prop;
    intersection.subsurface = false;
    return hit;
}

bool BVH_wrapper::intersect_nsf(Ray& ray, Worker& worker, Intersection& intersection) const {
    auto& node_stack = worker.node_stack();

    bool hit = false;

    uint32_t prop = 0xFFFFFFFF;

    node_stack.clear();
    if (0 != tree_.num_nodes_) {
        node_stack.push(0);
    }

    uint32_t n = 0;

    Simd3f const ray_origin(ray.origin.v);
    Simd3f const ray_inv_direction(ray.inv_direction.v);
    scalar const ray_min_t(ray.min_t());
    scalar       ray_max_t(ray.max_t());

    alignas(16) uint32_t ray_signs[4];
    sign(ray_inv_direction, ray_signs);

    bvh::Node* nodes = tree_.nodes_;

    Prop const* props = props_;

    uint32_t const* finite_props = tree_.indices_;

    while (!node_stack.empty()) {
        auto const& node = nodes[n];

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
                uint32_t const p = finite_props[i];
                if (props[p].intersect_nsf(p, ray, worker, intersection.geo)) {
                    prop      = p;
                    hit       = true;
                    ray_max_t = scalar(ray.max_t());
                }
            }
        }

        n = node_stack.pop();
    }

    uint32_t const* infinite_props = infinite_props_;

    for (uint32_t i = 0, len = num_infinite_props_; i < len; ++i) {
        uint32_t const p = infinite_props[i];
        if (props[p].intersect_nsf(p, ray, worker, intersection.geo)) {
            prop = p;
            hit  = true;
        }
    }

    intersection.prop = prop;
    //   intersection.subsurface = false;
    return hit;
}

bool BVH_wrapper::intersect(Ray& ray, Worker& worker, shape::Normals& normals) const {
    auto& node_stack = worker.node_stack();

    bool hit = false;

    node_stack.clear();
    if (0 != tree_.num_nodes_) {
        node_stack.push(0);
    }

    uint32_t n = 0;

    Simd3f const ray_origin(ray.origin.v);
    Simd3f const ray_inv_direction(ray.inv_direction.v);
    scalar const ray_min_t(ray.min_t());
    scalar       ray_max_t(ray.max_t());

    alignas(16) uint32_t ray_signs[4];
    sign(ray_inv_direction, ray_signs);

    bvh::Node* nodes = tree_.nodes_;

    Prop const* props = props_;

    uint32_t const* finite_props = tree_.indices_;

    while (!node_stack.empty()) {
        auto const& node = nodes[n];

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
                uint32_t const p = finite_props[i];
                if (props[p].intersect(p, ray, worker, normals)) {
                    hit       = true;
                    ray_max_t = scalar(ray.max_t());
                }
            }
        }

        n = node_stack.pop();
    }

    uint32_t const* infinite_props = infinite_props_;

    for (uint32_t i = 0, len = num_infinite_props_; i < len; ++i) {
        uint32_t const p = infinite_props[i];
        if (props[p].intersect(p, ray, worker, normals)) {
            hit = true;
        }
    }

    return hit;
}

bool BVH_wrapper::intersect_p(Ray const& ray, Worker& worker) const {
    auto& node_stack = worker.node_stack();

    node_stack.clear();
    if (0 != tree_.num_nodes_) {
        node_stack.push(0);
    }

    uint32_t n = 0;

    Simd3f const ray_origin(ray.origin.v);
    Simd3f const ray_inv_direction(ray.inv_direction.v);
    scalar const ray_min_t(ray.min_t());
    scalar       ray_max_t(ray.max_t());

    alignas(16) uint32_t ray_signs[4];
    sign(ray_inv_direction, ray_signs);

    bvh::Node* nodes = tree_.nodes_;

    Prop const* props = props_;

    uint32_t const* finite_props = tree_.indices_;

    while (!node_stack.empty()) {
        auto const& node = nodes[n];

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
                uint32_t const p = finite_props[i];
                if (props[p].intersect_p(p, ray, worker)) {
                    return true;
                }
            }
        }

        n = node_stack.pop();
    }

    uint32_t const* infinite_props = infinite_props_;

    for (uint32_t i = 0, len = num_infinite_props_; i < len; ++i) {
        uint32_t const p = infinite_props[i];
        if (props[p].intersect_p(p, ray, worker)) {
            return true;
        }
    }

    return false;
}

Result1 BVH_wrapper::visibility(Ray const& ray, Filter filter, Worker& worker) const {
    auto& node_stack = worker.node_stack();

    node_stack.clear();
    if (0 != tree_.num_nodes_) {
        node_stack.push(0);
    }

    uint32_t n = 0;

    float visibility = 1.f;

    Simd3f const ray_origin(ray.origin.v);
    Simd3f const ray_inv_direction(ray.inv_direction.v);
    scalar const ray_min_t(ray.min_t());
    scalar       ray_max_t(ray.max_t());

    alignas(16) uint32_t ray_signs[4];
    sign(ray_inv_direction, ray_signs);

    bvh::Node* nodes = tree_.nodes_;

    Prop const* props = props_;

    uint32_t const* finite_props = tree_.indices_;

    while (!node_stack.empty()) {
        auto& node = nodes[n];

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
                uint32_t const p = finite_props[i];
                visibility *= props[p].visibility(p, ray, filter, worker);
                if (visibility <= 0.f) {
                    return {false, 0.f};
                }
            }
        }

        n = node_stack.pop();
    }

    uint32_t const* infinite_props = infinite_props_;

    for (uint32_t i = 0, len = num_infinite_props_; i < len; ++i) {
        uint32_t const p = infinite_props[i];
        visibility *= props[p].visibility(p, ray, filter, worker);
        if (visibility <= 0.f) {
            return {false, 0.f};
        }
    }

    return {true, visibility};
}

bool BVH_wrapper::thin_absorption(Ray const& ray, Filter filter, Worker& worker, float3& ta) const {
    auto& node_stack = worker.node_stack();

    node_stack.clear();
    if (0 != tree_.num_nodes_) {
        node_stack.push(0);
    }

    uint32_t n = 0;

    float3 absorption(1.f);

    Simd3f const ray_origin(ray.origin.v);
    Simd3f const ray_inv_direction(ray.inv_direction.v);
    scalar const ray_min_t(ray.min_t());
    scalar       ray_max_t(ray.max_t());

    alignas(16) uint32_t ray_signs[4];
    sign(ray_inv_direction, ray_signs);

    bvh::Node* nodes = tree_.nodes_;

    Prop const* props = props_;

    uint32_t const* finite_props = tree_.indices_;

    while (!node_stack.empty()) {
        auto& node = nodes[n];

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
                uint32_t const p = finite_props[i];

                float3 tta;
                if (!props[p].thin_absorption(p, ray, filter, worker, tta)) {
                    return false;
                }

                absorption *= tta;
            }
        }

        n = node_stack.pop();
    }

    uint32_t const* infinite_props = infinite_props_;

    for (uint32_t i = 0, len = num_infinite_props_; i < len; ++i) {
        uint32_t const p = infinite_props[i];

        float3 tta;
        if (!props[p].thin_absorption(p, ray, filter, worker, tta)) {
            return false;
        }

        absorption *= tta;
    }

    ta = absorption;
    return true;
}

}  // namespace scene::prop
