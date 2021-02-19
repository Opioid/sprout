#ifndef SU_CORE_SCENE_LIGHT_TREE_HPP
#define SU_CORE_SCENE_LIGHT_TREE_HPP

#include "base/math/distribution/distribution_1d.hpp"
#include "base/math/vector.hpp"
#include "base/math/vector4.hpp"
#include "base/memory/array.hpp"

namespace math {
struct AABB;
}

namespace thread {
class Pool;
}

using Threads = thread::Pool;

namespace scene {

class Scene;

namespace shape::triangle {
struct Part;
}

namespace light {

class Light;
using Light_pick = Distribution_1D::Discrete;
using UInts      = uint32_t const* const;

struct Node {
    float weight(float3_p p, float3_p n, bool total_sphere) const;

    float weight(float3_p p0, float3_p p1, float3_p dir) const;

    bool split(float3_p p) const;

    bool split(float3_p p0, float3_p dir) const;

    template <typename Set>
    Light_pick random_light(float3_p p, float3_p n, bool total_sphere, float random,
                            UInts light_mapping, Set const& set) const;

    Light_pick random_light(float3_p p0, float3_p p1, float3_p dir, float random,
                            UInts light_mapping, Scene const& scene) const;

    template <typename Set>
    float pdf(float3_p p, float3_p n, bool total_sphere, uint32_t id,
              uint32_t const* const light_mapping, Set const& set) const;

    float4 center;
    float4 cone;

    float power;
    float variance;

    uint32_t has_children : 1;
    uint32_t two_sided : 1;
    uint32_t children_or_light : 30;
    uint32_t num_lights;
};

class Tree {
  public:
    static uint32_t constexpr Max_split_depth = 5;

    // (Max_split_depth + 1) to have space for worst case where we want to split on a leaf node,
    // which can have up to 4 lights
    static uint32_t constexpr Max_lights = 1 << (Max_split_depth + 1);

    static void set_splitting_threshold(float st);

    static float splitting_threshold_;

    using Lights = memory::Static_array<Light_pick, Max_lights>;

    Tree();

    ~Tree();

    void random_light(float3_p p, float3_p n, bool total_sphere, float random, bool split,
                      Scene const& scene, Lights& lights) const;

    void random_light(float3_p p0, float3_p p1, float random, bool split, Scene const& scene,
                      Lights& lights) const;

    float pdf(float3_p p, float3_p n, bool total_sphere, bool split, uint32_t id,
              Scene const& scene) const;

    void allocate_light_mapping(uint32_t num_lights);

    void allocate_nodes(uint32_t num_nodes);

    void allocate(uint32_t num_infinite_lights);

    float infinite_weight_;
    float infinite_guard_;

    uint32_t infinite_end_;
    uint32_t infinite_depth_bias_;
    uint32_t num_lights_;
    uint32_t num_infinite_lights_;
    uint32_t num_nodes_;

    Node*     nodes_;
    uint32_t* node_middles_;

    uint32_t* light_orders_;
    uint32_t* light_mapping_;

    float* infinite_light_powers_;

    Distribution_1D infinite_light_distribution_;
};

class Primitive_tree {
  public:
    using Part = shape::triangle::Part;

    Primitive_tree();

    ~Primitive_tree();

    Light_pick random_light(float3_p p, float3_p n, bool total_sphere, float random,
                            Part const& part) const;

    float pdf(float3_p p, float3_p n, bool total_sphere, uint32_t id, Part const& part) const;

    void allocate_light_mapping(uint32_t num_lights);

    void allocate_nodes(uint32_t num_nodes);

    uint32_t num_lights_;
    uint32_t num_nodes_;

    Node*     nodes_;
    uint32_t* node_middles_;

    uint32_t* light_orders_;
    uint32_t* light_mapping_;

    Distribution_1D* distributions_;
};

}  // namespace light
}  // namespace scene

#endif
