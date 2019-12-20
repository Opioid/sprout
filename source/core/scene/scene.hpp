#ifndef SU_CORE_SCENE_SCENE_HPP
#define SU_CORE_SCENE_SCENE_HPP

#include "base/math/distribution/distribution_1d.hpp"
#include "base/memory/array.hpp"
#include "bvh/scene_bvh_builder.hpp"
#include "light/light.hpp"
#include "light/light_tree.hpp"
#include "material/material.hpp"
#include "prop/prop_bvh_wrapper.hpp"
#include "resource/resource.hpp"
#include "scene_constants.hpp"
#include "shape/null.hpp"

#include <map>
#include <vector>

namespace resource {

template <typename T>
struct Resource_ptr;

}

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
struct Prop_ptr;

}  // namespace prop

class Extension;
class Worker;

struct Ray;

class Scene {
  public:
    using Node_stack     = shape::Node_stack;
    using Filter         = material::Sampler_settings::Filter;
    using Entity         = entity::Entity;
    using Entity_ref     = entity::Entity_ref;
    using Transformation = entity::Composed_transformation;
    using Keyframe       = entity::Keyframe;
    using Prop           = prop::Prop;
    using Prop_topology  = prop::Prop_topology;
    using Prop_ptr       = prop::Prop_ptr;
    using Material       = material::Material;
    using Shape          = shape::Shape;
    using Shape_ptr      = resource::Resource_ptr<Shape>;
    using Material_ptr   = resource::Resource_ptr<Material>;
    using Texture        = image::texture::Texture;

    Scene(Shape_ptr null_shape, std::vector<Shape*> const& shape_resources,
          std::vector<Material*> const& material_resources,
          std::vector<Texture*> const&  texture_resources) noexcept;

    ~Scene() noexcept;

    void clear() noexcept;

    void finish() noexcept;

    AABB const& aabb() const noexcept;

    AABB caustic_aabb() const noexcept;

    bool is_infinite() const noexcept;

    bool has_volumes() const noexcept;

    bool intersect(Ray& ray, Worker const& worker, prop::Intersection& intersection) const noexcept;

    bool intersect(Ray& ray, Worker const& worker, shape::Normals& normals) const noexcept;

    bool intersect_volume(Ray& ray, Worker const& worker, prop::Intersection& intersection) const
        noexcept;

    bool intersect_p(Ray const& ray, Worker const& worker) const noexcept;

    bool visibility(Ray const& ray, Filter filter, Worker const& worker, float& v) const noexcept;

    bool thin_absorption(Ray const& ray, Filter filter, Worker const& worker, float3& ta) const
        noexcept;

    Prop const* prop(uint32_t index) const noexcept;
    Prop*       prop(uint32_t index) noexcept;

    Prop* prop(std::string_view name) noexcept;

    std::vector<light::Light> const& lights() const noexcept;

    struct Light {
        light::Light const& ref;

        uint32_t id;
        float    pdf;
    };
    Light light(uint32_t id, bool calculate_pdf = true) const noexcept;

    Light light(uint32_t id, float3 const& p, float3 const& n, bool total_sphere,
                bool calculate_pdf = true) const noexcept;

    Light random_light(float random) const noexcept;

    Light random_light(float3 const& p, float3 const& n, bool total_sphere, float random) const
        noexcept;

    void simulate(uint64_t start, uint64_t end, thread::Pool& threads) noexcept;

    void compile(uint64_t time, thread::Pool& threads) noexcept;

    void calculate_num_interpolation_frames(uint64_t frame_step, uint64_t frame_duration) noexcept;

    uint32_t create_dummy() noexcept;
    uint32_t create_dummy(std::string const& name) noexcept;

    uint32_t create_prop(Shape_ptr shape, Material_ptr const* materials) noexcept;

    uint32_t create_prop(Shape_ptr shape, Material_ptr const* materials,
                         std::string const& name) noexcept;

    void create_prop_light(uint32_t prop, uint32_t part) noexcept;

    void create_prop_image_light(uint32_t prop, uint32_t part) noexcept;

    void create_prop_volume_light(uint32_t prop, uint32_t part) noexcept;

    void create_prop_volume_image_light(uint32_t prop, uint32_t part) noexcept;

    uint32_t create_extension(Extension* extension) noexcept;
    uint32_t create_extension(Extension* extension, std::string const& name) noexcept;

    void prop_serialize_child(uint32_t parent_id, uint32_t child_id) noexcept;

    void prop_set_transformation(uint32_t entity, math::Transformation const& t) noexcept;

    // Only the returned reference is guaranteed to contain the actual transformation data.
    // This might or might not be the same reference which is passed as a parameter,
    // depending on whether the entity is animated or not.
    // This can sometimes avoid a relatively costly copy,
    // while keeping the animated state out of the interface.
    Transformation const& prop_transformation_at(uint32_t entity, uint64_t time,
                                                 Transformation& transformation) const noexcept;

    Transformation const& prop_transformation_at(uint32_t entity, uint64_t time, bool is_static,
                                                 Transformation& transformation) const noexcept;

    Transformation const& prop_world_transformation(uint32_t entity) const noexcept;

    math::Transformation const& prop_local_frame_0(uint32_t entity) const noexcept;

    void prop_set_world_transformation(uint32_t entity, math::Transformation const& t) noexcept;

    void prop_allocate_frames(uint32_t entity, bool local_animation) noexcept;

    bool prop_has_animated_frames(uint32_t entity) const noexcept;

    void prop_set_frames(uint32_t entity, animation::Keyframe const* frames) noexcept;

    void prop_calculate_world_transformation(uint32_t entity) noexcept;

    void prop_propagate_transformation(uint32_t entity) noexcept;

    void prop_inherit_transformation(uint32_t              entity,
                                     const Transformation& transformation) noexcept;

    void prop_inherit_transformation(uint32_t entity, entity::Keyframe const* frames) noexcept;

    void prop_set_visibility(uint32_t entity, bool in_camera, bool in_reflection,
                             bool in_shadow) noexcept;

    void prop_prepare_sampling(uint32_t entity, uint32_t part, uint32_t light_id, uint64_t time,
                               bool material_importance_sampling, bool volume,
                               thread::Pool& threads) noexcept;

    AABB const& prop_aabb(uint32_t entity) const noexcept;

    bool prop_aabb_intersect_p(uint32_t entity, Ray const& ray) const noexcept;

    Shape* prop_shape(uint32_t entity) const noexcept;

    material::Material const* prop_material(uint32_t entity, uint32_t part) const noexcept;

    prop::Prop_topology const& prop_topology(uint32_t entity) const noexcept;

    Texture const* texture(uint32_t id) const noexcept;

    uint32_t prop_light_id(uint32_t entity, uint32_t part) const noexcept;

    float light_area(uint32_t entity, uint32_t part) const noexcept;

    float3 light_center(uint32_t light) const noexcept;

    animation::Animation* create_animation(uint32_t count) noexcept;

    void create_animation_stage(uint32_t entity, animation::Animation* animation) noexcept;

    size_t num_bytes() const noexcept;

  private:
    void prop_animated_transformation_at(uint32_t entity, uint64_t time,
                                         Transformation& transformation) const noexcept;

    Prop_ptr allocate_prop() noexcept;

    void allocate_light(light::Light::Type type, uint32_t entity, uint32_t part) noexcept;

    bool prop_is_instance(Shape_ptr shape, Material_ptr const* materials, uint32_t num_parts) const
        noexcept;

    bool prop_has_caustic_material(uint32_t entity) const noexcept;

    void add_named_prop(uint32_t prop, std::string const& name) noexcept;

    uint32_t count_frames(uint64_t frame_step, uint64_t frame_duration) const noexcept;

    uint64_t const tick_duration_ = Units_per_second / 60;

    uint32_t num_interpolation_frames_ = 0;

    bvh::Builder bvh_builder_;

    prop::BVH_wrapper prop_bvh_;
    prop::BVH_wrapper volume_bvh_;

    Shape_ptr null_shape_;

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
    std::vector<float3>       light_centers_;

    std::vector<uint32_t> materials_;
    std::vector<uint32_t> light_ids_;
    std::vector<Keyframe> keyframes_;

    std::vector<animation::Animation*> animations_;

    std::vector<animation::Stage> animation_stages_;

    std::map<std::string, uint32_t, std::less<>> named_props_;

    memory::Array<float> light_powers_;

    Distribution_implicit_pdf_lut_lin_1D light_distribution_;

    light::Tree light_tree_;

    std::vector<Shape*> const& shape_resources_;

    std::vector<Material*> const& material_resources_;

    std::vector<Texture*> const& texture_resources_;

    std::vector<uint32_t> finite_props_;
    std::vector<uint32_t> infinite_props_;

    std::vector<uint32_t> volumes_;
    std::vector<uint32_t> infinite_volumes_;

    std::vector<Extension*> extensions_;
};

}  // namespace scene

#endif
