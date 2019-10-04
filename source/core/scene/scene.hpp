#ifndef SU_CORE_SCENE_SCENE_HPP
#define SU_CORE_SCENE_SCENE_HPP

#include <map>
#include <vector>
#include "base/math/distribution/distribution_1d.hpp"
#include "base/memory/array.hpp"
#include "bvh/scene_bvh_builder.hpp"
#include "material/material.hpp"
#include "prop/prop_bvh_wrapper.hpp"
#include "scene_constants.hpp"
#include "shape/null.hpp"
#include "take/take_settings.hpp"

namespace thread {
class Pool;
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

}  // namespace entity

namespace light {

class Light;

}  // namespace light

namespace prop {

struct Intersection;
class Prop;
struct Prop_material;
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
    using Prop           = prop::Prop;
    using Prop_ptr       = prop::Prop_ptr;
    using Material       = material::Material;
    using Shape          = shape::Shape;

    using Materials = memory::Array<material::Material*>;

    Scene() noexcept;

    ~Scene() noexcept;

    void clear() noexcept;

    void finish() noexcept;

    AABB const& aabb() const noexcept;

    AABB caustic_aabb() const noexcept;
    AABB caustic_aabb(float3x3 const& rotation) const noexcept;

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

    Prop const* prop(size_t index) const noexcept;
    Prop*       prop(size_t index) noexcept;

    Prop* prop(std::string_view name) noexcept;

    //   std::vector<light::Light*> const& lights() const noexcept;
    std::vector<light::Light> const& lights() const noexcept;

    struct Light {
        light::Light const& ref;

        float    pdf;
        uint32_t id;
    };
    Light light(uint32_t id, bool calculate_pdf = true) const noexcept;

    Light random_light(float random) const noexcept;

    void simulate(uint64_t start, uint64_t end, thread::Pool& thread_pool) noexcept;

    void compile(uint64_t time, thread::Pool& pool) noexcept;

    void calculate_num_interpolation_frames(uint64_t frame_step, uint64_t frame_duration) noexcept;

    uint32_t create_dummy() noexcept;
    uint32_t create_dummy(std::string const& name) noexcept;

    uint32_t create_prop(Shape* shape, Materials const& materials) noexcept;

    uint32_t create_prop(Shape* shape, Materials const& materials,
                         std::string const& name) noexcept;

    void create_prop_light(uint32_t prop, uint32_t part) noexcept;

    void create_prop_image_light(uint32_t prop, uint32_t part) noexcept;

    void create_prop_volume_light(uint32_t prop, uint32_t part) noexcept;

    void create_prop_volume_image_light(uint32_t prop, uint32_t part) noexcept;

    uint32_t create_extension(Extension* extension) noexcept;
    uint32_t create_extension(Extension* extension, std::string const& name) noexcept;

    void prop_attach(uint32_t parent_id, uint32_t child_id, uint32_t sibling_hint) noexcept;

    void prop_set_transformation(uint32_t entity, math::Transformation const& t) noexcept;

    Transformation const& prop_world_transformation(uint32_t entity) const noexcept;

    void prop_set_world_transformation(uint32_t entity, math::Transformation const& t) noexcept;

    void prop_allocate_frames(uint32_t entity, uint32_t num_world_frames,
                              uint32_t num_local_frames) noexcept;

    void prop_set_frames(uint32_t entity, animation::Keyframe const* frames,
                         uint32_t num_frames) noexcept;

    entity::Morphing const& prop_morphing(uint32_t entity) const noexcept;

    void prop_set_morphing(uint32_t entity, entity::Morphing const& morphing) noexcept;

    void prop_set_visibility(uint32_t entity, bool in_camera, bool in_reflection,
                             bool in_shadow) noexcept;

    void prop_prepare_sampling(uint32_t entity, uint32_t part, uint32_t light_id, uint64_t time,
                               bool material_importance_sampling, thread::Pool& pool) noexcept;

    void prop_prepare_sampling_volume(uint32_t entity, uint32_t part, uint32_t light_id,
                                      uint64_t time, bool material_importance_sampling,
                                      thread::Pool& pool) noexcept;

    material::Material* const* prop_materials(uint32_t entity) const noexcept;

    material::Material const* prop_material(uint32_t entity, uint32_t part) const noexcept;

    prop::Prop_topology const& prop_topology(uint32_t entity) const noexcept;

    uint32_t prop_light_id(uint32_t entity, uint32_t part) const noexcept;

    float prop_area(uint32_t entity, uint32_t part) const noexcept;

    float prop_volume(uint32_t entity, uint32_t part) const noexcept;

    void add_material(Material* material) noexcept;

    animation::Animation* create_animation(uint32_t count) noexcept;

    void create_animation_stage(uint32_t entity, animation::Animation* animation) noexcept;

    size_t num_bytes() const noexcept;

  private:
    Prop_ptr allocate_prop() noexcept;

    void prop_add_sibling(uint32_t self, uint32_t node) noexcept;

    void prop_detach_self(uint32_t self) noexcept;

    void prop_detach(uint32_t self, uint32_t node) noexcept;

    void prop_remove_sibling(uint32_t self, uint32_t node) noexcept;

    bool prop_has_caustic_material(uint32_t entity, uint32_t num_parts) const noexcept;

    void add_named_prop(uint32_t prop, std::string const& name) noexcept;

    uint32_t count_frames(uint64_t frame_step, uint64_t frame_duration) const noexcept;

    uint64_t const tick_duration_ = Units_per_second / 60;

    uint32_t num_interpolation_frames_ = 0;

    bvh::Builder<prop::Prop> bvh_builder_;

    prop::BVH_wrapper prop_bvh_;
    prop::BVH_wrapper volume_bvh_;

    shape::Null null_shape_;

    bool has_masked_material_;
    bool has_tinted_shadow_;
    bool has_volumes_;

    std::vector<prop::Prop>          props_;
    std::vector<Transformation>      prop_world_transformations_;
    std::vector<entity::Morphing>    prop_morphing_;
    std::vector<prop::Prop_material> prop_materials_;
    std::vector<prop::Prop_topology> prop_topology_;

    std::vector<uint32_t> finite_props_;
    std::vector<uint32_t> infinite_props_;

    std::vector<uint32_t> volumes_;
    std::vector<uint32_t> infinite_volumes_;

    // std::vector<light::Light*> lights_;
    std::vector<light::Light> lights_;

    std::vector<Extension*> extensions_;

    std::map<std::string, uint32_t, std::less<>> named_props_;

    memory::Array<float> light_powers_;

    math::Distribution_implicit_pdf_lut_lin_1D light_distribution_;

    std::vector<Material*> materials_;

    std::vector<animation::Animation*> animations_;

    std::vector<animation::Stage> animation_stages_;
};

}  // namespace scene

#endif
