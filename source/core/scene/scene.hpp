#ifndef SU_CORE_SCENE_SCENE_HPP
#define SU_CORE_SCENE_SCENE_HPP

#include "base/math/distribution/distribution_1d.hpp"
#include "base/memory/array.hpp"
#include "bvh/scene_bvh_builder.hpp"
#include "light/light.hpp"
#include "light/light_tree.hpp"
#include "light/light_tree_builder.hpp"
#include "material/material.hpp"
#include "prop/prop_bvh_wrapper.hpp"
#include "scene_constants.hpp"
#include "shape/null.hpp"

#include <vector>

namespace thread {
class Pool;
}

namespace image::texture {
class Texture;
}

namespace scene {

namespace shape {
class Shape;
}  // namespace shape

namespace animation {

struct Keyframe;
class Animation;
class Stage;

}  // namespace animation

namespace entity {

class Entity;
struct Entity_ref;
class Dummy;
struct Morphing;
struct Keyframe;

}  // namespace entity

namespace prop {

struct Intersection;
class Prop;
struct Prop_part;
struct Prop_frames;
struct Prop_topology;

}  // namespace prop

class Extension;
class Worker;

struct Ray;

struct Light_select {
    light::Light const& ref;

    uint32_t offset;
    float    pdf;
};

class Scene {
  public:
    using Node_stack     = shape::Node_stack;
    using Filter         = material::Sampler_settings::Filter;
    using Entity         = entity::Entity;
    using Entity_ref     = entity::Entity_ref;
    using Transformation = entity::Composed_transformation;
    using Keyframe       = entity::Keyframe;
    using Light_pick     = light::Light_pick;
    using Lights         = light::Tree::Lights;
    using Intersection   = prop::Intersection;
    using Prop           = prop::Prop;
    using Prop_topology  = prop::Prop_topology;
    using Material       = material::Material;
    using Shape          = shape::Shape;
    using Texture        = image::texture::Texture;

    Scene(uint32_t null_shape, std::vector<Shape*> const& shape_resources,
          std::vector<Material*> const& material_resources,
          std::vector<Texture*> const&  texture_resources);

    ~Scene();

    void clear();

    AABB aabb() const;

    AABB caustic_aabb() const;

    bool is_infinite() const;

    bool has_volumes() const;

    bool intersect(Ray& ray, Worker& worker, Intersection& isec) const;

    bool intersect(Ray& ray, Worker& worker, shape::Normals& normals) const;

    bool intersect_volume(Ray& ray, Worker& worker, Intersection& isec) const;

    Result1 visibility(Ray const& ray, Filter filter, Worker& worker) const;

    bool tinted_visibility(Ray const& ray, Filter filter, Worker& worker, float3& ta) const;

    uint32_t num_props() const;

    Prop const* prop(uint32_t index) const;

    Prop* prop(uint32_t index);

    Shape const* shape(uint32_t index) const;

    Material const* material(uint32_t index) const;

    uint32_t num_lights() const;

    light::Light const& light(uint32_t id) const;

    Light_select light(uint32_t id, float3_p p, float3_p n, bool total_sphere, bool split) const;

    Light_select light(uint32_t id, float3_p p0, float3_p p1, bool split) const;

    Light_pick random_light(float random) const;

    void random_light(float3_p p, float3_p n, bool total_sphere, float random, bool split,
                      Lights& lights) const;

    void random_light(float3_p p0, float3_p p1, float random, bool split, Lights& lights) const;

    void simulate(uint64_t start, uint64_t end, Threads& threads);

    void compile(uint64_t time, Threads& threads);

    uint32_t num_interpolation_frames() const;

    void calculate_num_interpolation_frames(uint64_t frame_step, uint64_t frame_duration);

    uint32_t create_entity();

    uint32_t create_prop(uint32_t shape, uint32_t const* materials);

    void create_light(uint32_t prop);

    uint32_t create_extension(Extension* extension);

    void prop_serialize_child(uint32_t parent_id, uint32_t child_id);

    void prop_set_transformation(uint32_t entity, math::Transformation const& t);

    void prop_set_world_transformation(uint32_t entity, math::Transformation const& t);

    // Only the returned reference is guaranteed to contain the actual trafo data.
    // This might or might not be the same reference which is passed as a parameter,
    // depending on whether the entity is animated or not.
    // This can sometimes avoid a relatively costly copy,
    // while keeping the animated state out of the interface.
    Transformation const& prop_transformation_at(uint32_t entity, uint64_t time,
                                                 Transformation& trafo) const;

    Transformation const& prop_transformation_at(uint32_t entity, uint64_t time, bool is_static,
                                                 Transformation& trafo) const;

    Transformation const& prop_world_transformation(uint32_t entity) const;

    math::Transformation const& prop_local_frame_0(uint32_t entity) const;

    void prop_allocate_frames(uint32_t entity, bool local_animation);

    bool prop_has_animated_frames(uint32_t entity) const;

    void prop_set_frames(uint32_t entity, animation::Keyframe const* frames);

    void prop_set_frame(uint32_t entity, uint32_t frame, Keyframe const& k);

    void prop_calculate_world_transformation(uint32_t entity);

    void prop_propagate_transformation(uint32_t entity);

    void prop_inherit_transformation(uint32_t entity, const Transformation& trafo);

    void prop_inherit_transformation(uint32_t entity, Keyframe const* frames);

    void prop_set_visibility(uint32_t entity, bool in_camera, bool in_reflection, bool in_shadow);

    void prop_prepare_sampling(uint32_t entity, uint32_t part, uint32_t light_id, uint64_t time,
                               bool material_importance_sampling, bool volume, Threads& threads);

    AABB const& prop_aabb(uint32_t entity) const;

    bool prop_aabb_intersect_p(uint32_t entity, Ray const& ray) const;

    Shape* prop_shape(uint32_t entity) const;

    material::Material const* prop_material(uint32_t entity, uint32_t part) const;

    uint32_t prop_material_id(uint32_t entity, uint32_t part) const;

    prop::Prop_topology const& prop_topology(uint32_t entity) const;

    Texture const* texture(uint32_t id) const;

    uint32_t prop_light_id(uint32_t entity, uint32_t part) const;

    float light_area(uint32_t entity, uint32_t part) const;

    bool light_two_sided(uint32_t entity) const;

    float light_power(uint32_t light) const;

    AABB light_aabb(uint32_t light) const;

    float4 light_cone(uint32_t light) const;

    animation::Animation* create_animation(uint32_t count);

    void create_animation_stage(uint32_t entity, animation::Animation* animation);

  private:
    Transformation const& prop_animated_transformation_at(uint32_t frames, uint64_t time,
                                                          Transformation& trafo) const;

    struct Prop_ptr {
        Prop* ptr;

        uint32_t id;
    };
    Prop_ptr allocate_prop();

    void allocate_light(light::Light::Type type, bool two_sided, uint32_t entity, uint32_t part);

    bool prop_is_instance(uint32_t shape, uint32_t const* materials, uint32_t num_parts) const;

    bool prop_has_caustic_material(uint32_t entity) const;

    uint32_t count_frames(uint64_t frame_step, uint64_t frame_duration) const;

    uint64_t const tick_duration_ = Units_per_second / 60;

    uint32_t num_interpolation_frames_ = 0;

    uint64_t current_time_start_;

    bvh::Builder bvh_builder_;

    light::Tree_builder light_tree_builder_;

    prop::BVH_wrapper prop_bvh_;
    prop::BVH_wrapper volume_bvh_;

    AABB caustic_aabb_;

    uint32_t null_shape_;

    bool has_masked_material_;
    bool has_tinted_shadow_;
    bool has_volumes_;

    std::vector<Prop>           props_;
    std::vector<Transformation> prop_world_transformations_;
    std::vector<uint32_t>       prop_parts_;
    std::vector<uint32_t>       prop_frames_;
    std::vector<Prop_topology>  prop_topology_;
    // Pre-transformed AABB in world space.
    // For moving objects it must cover the entire area occupied by the object during the tick.
    std::vector<AABB> prop_aabbs_;

    std::vector<light::Light> lights_;
    std::vector<AABB>         light_aabbs_;
    std::vector<float4>       light_cones_;

    std::vector<uint32_t> materials_;
    std::vector<uint32_t> light_ids_;
    std::vector<Keyframe> keyframes_;

    std::vector<animation::Animation*> animations_;

    std::vector<animation::Stage> animation_stages_;

    memory::Array<float> light_temp_powers_;

    Distribution_1D light_distribution_;

    light::Tree light_tree_;

    std::vector<Shape*> const&    shape_resources_;
    std::vector<Material*> const& material_resources_;
    std::vector<Texture*> const&  texture_resources_;

    std::vector<uint32_t> finite_props_;
    std::vector<uint32_t> infinite_props_;

    std::vector<uint32_t> volumes_;
    std::vector<uint32_t> infinite_volumes_;

    std::vector<Extension*> extensions_;
};

}  // namespace scene

#endif
