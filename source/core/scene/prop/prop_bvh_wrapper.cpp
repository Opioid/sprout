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

AABB BVH_wrapper::aabb() const {
    if (!tree_.nodes_) {
        return Empty_AABB;
    }

    return tree_.nodes_[0].aabb();
}

bool BVH_wrapper::intersect(Ray& ray, Worker& worker, Interpolation ipo, Intersection& isec) const {
    auto& stack = worker.node_stack();

    bool hit = false;

    uint32_t prop = prop::Null;

    stack.clear();
    if (0 != tree_.num_nodes_) {
        stack.push(0);
    }

    uint32_t n = 0;

    Simdf const  ray_origin(ray.origin.v);
    Simdf const  ray_inv_direction(ray.inv_direction.v);
    scalar const ray_min_t(ray.min_t());
    scalar       ray_max_t(ray.max_t());

    alignas(16) uint32_t ray_signs[4];
    sign(ray_inv_direction, ray_signs);

    bvh::Node* nodes = tree_.nodes_;

    Prop const* props = props_;

    uint32_t const* finite_props = tree_.indices_;

    while (!stack.empty()) {
        auto const& node = nodes[n];

        if (node.intersect_p(ray_origin, ray_inv_direction, ray_min_t, ray_max_t)) {
            if (0 == node.num_indices()) {
                uint32_t const a = node.children();
                uint32_t const b = a + 1;

                if (0 == ray_signs[node.axis()]) {
                    stack.push(b);
                    n = a;
                } else {
                    stack.push(a);
                    n = b;
                }

                continue;
            }

            for (uint32_t i = node.indices_start(), len = node.indices_end(); i < len; ++i) {
                uint32_t const p = finite_props[i];
                if (props[p].intersect(p, ray, worker, ipo, isec.geo)) {
                    prop      = p;
                    hit       = true;
                    ray_max_t = scalar(ray.max_t());
                }
            }
        }

        n = stack.pop();
    }

    uint32_t const* infinite_props = infinite_props_;

    for (uint32_t i = 0, len = num_infinite_props_; i < len; ++i) {
        uint32_t const p = infinite_props[i];
        if (props[p].intersect(p, ray, worker, ipo, isec.geo)) {
            prop = p;
            hit  = true;
        }
    }

    isec.prop       = prop;
    isec.subsurface = false;
    return hit;
}

bool BVH_wrapper::intersect_shadow(Ray& ray, Worker& worker, Intersection& isec) const {
    auto& stack = worker.node_stack();

    bool hit = false;

    uint32_t prop = prop::Null;

    stack.clear();
    if (0 != tree_.num_nodes_) {
        stack.push(0);
    }

    uint32_t n = 0;

    Simdf const  ray_origin(ray.origin.v);
    Simdf const  ray_inv_direction(ray.inv_direction.v);
    scalar const ray_min_t(ray.min_t());
    scalar       ray_max_t(ray.max_t());

    alignas(16) uint32_t ray_signs[4];
    sign(ray_inv_direction, ray_signs);

    bvh::Node* nodes = tree_.nodes_;

    Prop const* props = props_;

    uint32_t const* finite_props = tree_.indices_;

    while (!stack.empty()) {
        auto const& node = nodes[n];

        if (node.intersect_p(ray_origin, ray_inv_direction, ray_min_t, ray_max_t)) {
            if (0 == node.num_indices()) {
                uint32_t const a = node.children();
                uint32_t const b = a + 1;

                if (0 == ray_signs[node.axis()]) {
                    stack.push(b);
                    n = a;
                } else {
                    stack.push(a);
                    n = b;
                }

                continue;
            }

            for (uint32_t i = node.indices_start(), len = node.indices_end(); i < len; ++i) {
                uint32_t const p = finite_props[i];
                if (props[p].intersect_shadow(p, ray, worker, isec.geo)) {
                    prop      = p;
                    hit       = true;
                    ray_max_t = scalar(ray.max_t());
                }
            }
        }

        n = stack.pop();
    }

    uint32_t const* infinite_props = infinite_props_;

    for (uint32_t i = 0, len = num_infinite_props_; i < len; ++i) {
        uint32_t const p = infinite_props[i];
        if (props[p].intersect_shadow(p, ray, worker, isec.geo)) {
            prop = p;
            hit  = true;
        }
    }

    isec.prop       = prop;
    isec.subsurface = false;
    return hit;
}

bool BVH_wrapper::intersect_p(Ray const& ray, Worker& worker) const {
    auto& stack = worker.node_stack();

    stack.clear();
    if (0 != tree_.num_nodes_) {
        stack.push(0);
    }

    uint32_t n = 0;

    Simdf const  ray_origin(ray.origin.v);
    Simdf const  ray_inv_direction(ray.inv_direction.v);
    scalar const ray_min_t(ray.min_t());
    scalar       ray_max_t(ray.max_t());

    alignas(16) uint32_t ray_signs[4];
    sign(ray_inv_direction, ray_signs);

    bvh::Node* nodes = tree_.nodes_;

    Prop const* props = props_;

    uint32_t const* finite_props = tree_.indices_;

    while (!stack.empty()) {
        auto const& node = nodes[n];

        if (node.intersect_p(ray_origin, ray_inv_direction, ray_min_t, ray_max_t)) {
            if (0 == node.num_indices()) {
                uint32_t const a = node.children();
                uint32_t const b = a + 1;

                if (0 == ray_signs[node.axis()]) {
                    stack.push(b);
                    n = a;
                } else {
                    stack.push(a);
                    n = b;
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

        n = stack.pop();
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

bool BVH_wrapper::visibility(Ray const& ray, Filter filter, Worker& worker, float3& vis) const {
    auto& stack = worker.node_stack();

    stack.clear();
    if (0 != tree_.num_nodes_) {
        stack.push(0);
    }

    uint32_t n = 0;

    float3 local_vis(1.f);

    Simdf const  ray_origin(ray.origin.v);
    Simdf const  ray_inv_direction(ray.inv_direction.v);
    scalar const ray_min_t(ray.min_t());
    scalar       ray_max_t(ray.max_t());

    alignas(16) uint32_t ray_signs[4];
    sign(ray_inv_direction, ray_signs);

    bvh::Node* nodes = tree_.nodes_;

    Prop const* props = props_;

    uint32_t const* finite_props = tree_.indices_;

    while (!stack.empty()) {
        auto const& node = nodes[n];

        if (node.intersect_p(ray_origin, ray_inv_direction, ray_min_t, ray_max_t)) {
            if (0 == node.num_indices()) {
                uint32_t const a = node.children();
                uint32_t const b = a + 1;

                if (0 == ray_signs[node.axis()]) {
                    stack.push(b);
                    n = a;
                } else {
                    stack.push(a);
                    n = b;
                }

                continue;
            }

            for (uint32_t i = node.indices_start(), len = node.indices_end(); i < len; ++i) {
                uint32_t const p = finite_props[i];

                float3 tv;
                if (!props[p].visibility(p, ray, filter, worker, tv)) {
                    return false;
                }

                local_vis *= tv;
            }
        }

        n = stack.pop();
    }

    uint32_t const* infinite_props = infinite_props_;

    for (uint32_t i = 0, len = num_infinite_props_; i < len; ++i) {
        uint32_t const p = infinite_props[i];

        float3 tv;
        if (!props[p].visibility(p, ray, filter, worker, tv)) {
            return false;
        }

        local_vis *= tv;
    }

    vis = local_vis;
    return true;
}

}  // namespace scene::prop
