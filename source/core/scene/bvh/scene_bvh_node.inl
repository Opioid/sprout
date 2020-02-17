#ifndef SU_CORE_SCENE_BVH_NODE_INL
#define SU_CORE_SCENE_BVH_NODE_INL

#include "base/math/ray.hpp"
#include "base/math/vector3.inl"
#include "scene_bvh_node.hpp"

namespace scene::bvh {

inline Node::Node() = default;

inline float3 Node::min() const {
    return float3(min_.v);
}

inline float3 Node::max() const {
    return float3(max_.v);
}

inline uint32_t Node::next() const {
    return min_.next_or_data;
}

inline uint8_t Node::axis() const {
    return max_.axis;
}

inline uint8_t Node::num_primitives() const {
    return max_.num_primitives;
}

inline uint32_t Node::indices_start() const {
    return min_.next_or_data;
}

inline uint32_t Node::indices_end() const {
    return min_.next_or_data + uint32_t(max_.num_primitives);
}

inline void Node::set_aabb(float const* min, float const* max) {
    min_.v[0] = min[0];
    min_.v[1] = min[1];
    min_.v[2] = min[2];

    max_.v[0] = max[0];
    max_.v[1] = max[1];
    max_.v[2] = max[2];
}

inline void Node::set_split_node(uint32_t next_node, uint8_t axis) {
    min_.next_or_data   = next_node;
    max_.axis           = axis;
    max_.num_primitives = 0;
}

inline void Node::set_leaf_node(uint32_t start_primitive, uint8_t num_primitives) {
    min_.next_or_data   = start_primitive;
    max_.num_primitives = num_primitives;
}

// This test is presented in the paper
// "An Efficient and Robust Ray–Box Intersection Algorithm"
// http://www.cs.utah.edu/~awilliam/box/box.pdf

/*
inline bool Node::intersect_p(math::ray const& ray) const {
        int8_t sign_0 = ray.signs[0];
        float min_t = (bounds[    sign_0].x - ray.origin.x) * ray.inv_direction.x;
        float max_t = (bounds[1 - sign_0].x - ray.origin.x) * ray.inv_direction.x;

        int8_t sign_1 = ray.signs[1];
        float min_ty = (bounds[    sign_1].y - ray.origin.y) * ray.inv_direction.y;
        float max_ty = (bounds[1 - sign_1].y - ray.origin.y) * ray.inv_direction.y;

        if (min_t > max_ty || min_ty > max_t) {
                return false;
        }

        if (min_ty > min_t) {
                min_t = min_ty;
        }

        if (max_ty < max_t) {
                max_t = max_ty;
        }

        int8_t sign_2 = ray.signs[2];
        float min_tz = (bounds[    sign_2].z - ray.origin.z) * ray.inv_direction.z;
        float max_tz = (bounds[1 - sign_2].z - ray.origin.z) * ray.inv_direction.z;

        if (min_t > max_tz || min_tz > max_t) {
                return false;
        }

        if (min_tz > min_t) {
                min_t = min_tz;
        }

        if (max_tz < max_t) {
                max_t = max_tz;
        }

        return min_t < ray.max_t && max_t > ray.min_t;
}
*/

// I found this SSE optimized AABB/ray test here:
// http://www.flipcode.com/archives/SSE_RayBox_Intersection_Test.shtml
inline bool Node::intersect_p(Simd3f const& ray_origin, Simd3f const& ray_inv_direction,
                              scalar const& ray_min_t, scalar const& ray_max_t) const {
    Simd3f const bb_min = Simd3f::create_from_3(min_.v);
    Simd3f const bb_max = Simd3f::create_from_3(max_.v);

    Simd3f const l1 = (bb_min - ray_origin) * ray_inv_direction;
    Simd3f const l2 = (bb_max - ray_origin) * ray_inv_direction;

    // the order we use for those min/max is vital to filter out
    // NaNs that happens when an inv_dir is +/- inf and
    // (box_min - pos) is 0. inf * 0 = NaN
    Simd3f const filtered_l1a = math::min(l1, Simd3f(simd::Infinity));
    Simd3f const filtered_l2a = math::min(l2, Simd3f(simd::Infinity));

    Simd3f const filtered_l1b = math::max(l1, Simd3f(simd::Neg_infinity));
    Simd3f const filtered_l2b = math::max(l2, Simd3f(simd::Neg_infinity));

    // now that we're back on our feet, test those slabs.
    Simd3f max_t = math::max(filtered_l1a, filtered_l2a);
    Simd3f min_t = math::min(filtered_l1b, filtered_l2b);

    // unfold back. try to hide the latency of the shufps & co.
    max_t = min_scalar(max_t, SU_ROTATE_LEFT(max_t.v));
    min_t = max_scalar(min_t, SU_ROTATE_LEFT(min_t.v));

    max_t = min_scalar(max_t, SU_MUX_HIGH(max_t.v, max_t.v));
    min_t = max_scalar(min_t, SU_MUX_HIGH(min_t.v, min_t.v));

    return 0 != (_mm_comige_ss(max_t.v, ray_min_t.v) & _mm_comige_ss(ray_max_t.v, min_t.v) &
                 _mm_comige_ss(max_t.v, min_t.v));
}

}  // namespace scene::bvh

#endif
