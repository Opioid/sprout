#ifndef SU_CORE_SCENE_SCENE_HPP
#define SU_CORE_SCENE_SCENE_HPP

#include <map>
#include <vector>
#include "base/math/distribution/distribution_1d.hpp"
#include "base/memory/array.hpp"
#include "bvh/scene_bvh_builder.hpp"
#include "light/null_light.hpp"
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

}  // namespace entity

namespace light {

class Light;
class Prop_light;
class Prop_image_light;
class Prop_volume_light;
class Prop_volume_image_light;

}  // namespace light

namespace prop {

struct Intersection;
class Prop;
struct Prop_ref;

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
    using Prop_ref       = prop::Prop_ref;
    using Material       = material::Material;
    using Shape          = shape::Shape;

    using Materials = memory::Array<material::Material*>;

    Scene() noexcept;

    ~Scene() noexcept;

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

    Prop* prop(std::string_view name) const noexcept;

    std::vector<light::Light*> const& lights() const noexcept;

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

    Prop_ref create_dummy() noexcept;
    Prop_ref create_dummy(std::string const& name) noexcept;

    Prop_ref create_prop(Shape* shape, Materials const& materials) noexcept;

    Prop_ref create_prop(Shape* shape, Materials const& materials,
                         std::string const& name) noexcept;

    light::Light* create_prop_light(uint32_t prop, uint32_t part) noexcept;

    light::Light* create_prop_image_light(uint32_t prop, uint32_t part) noexcept;

    light::Light* create_prop_volume_light(uint32_t prop, uint32_t part) noexcept;

    light::Light* create_prop_volume_image_light(uint32_t prop, uint32_t part) noexcept;

    Prop_ref create_extension(Extension* extension) noexcept;
    //   uint32_t create_extension(Extension* extension, std::string const& name) noexcept;

    void prop_attach(uint32_t parent_id, uint32_t child_id) noexcept;

    void prop_set_transformation(uint32_t entity, math::Transformation const& t) noexcept;

    Transformation const& prop_world_transformation(uint32_t entity) const noexcept;
    void prop_set_world_transformation(uint32_t entity, math::Transformation const& t) noexcept;

    void prop_allocate_frames(uint32_t entity, uint32_t num_world_frames,
                              uint32_t num_local_frames) noexcept;

    void prop_set_frames(uint32_t entity, animation::Keyframe const* frames,
                         uint32_t num_frames) noexcept;

    void add_material(Material* material) noexcept;

    animation::Animation* create_animation(uint32_t count) noexcept;

    void create_animation_stage(uint32_t entity, animation::Animation* animation) noexcept;

    size_t num_bytes() const noexcept;

  private:
    void add_named_prop(Prop* prop, std::string const& name) noexcept;

    uint32_t count_frames(uint64_t frame_step, uint64_t frame_duration) const noexcept;

    uint64_t const tick_duration_ = Units_per_second / 60;

    uint32_t num_interpolation_frames_ = 0;

    bvh::Builder<prop::Prop> bvh_builder_;

    prop::BVH_wrapper prop_bvh_;
    prop::BVH_wrapper volume_bvh_;

    shape::Null       null_shape_;
    light::Null_light null_light_;

    bool has_masked_material_;
    bool has_tinted_shadow_;
    bool has_volumes_;

    std::vector<prop::Prop>     props_;
    std::vector<Transformation> world_transformations_;

    std::vector<uint32_t> finite_props_;
    std::vector<uint32_t> infinite_props_;

    std::vector<uint32_t> volumes_;
    std::vector<uint32_t> infinite_volumes_;

    std::vector<light::Light*> lights_;

    std::vector<Extension*> extensions_;

    std::map<std::string, Prop*, std::less<>> named_props_;

    memory::Array<float> light_powers_;

    math::Distribution_implicit_pdf_lut_lin_1D light_distribution_;

    std::vector<Material*> materials_;

    std::vector<animation::Animation*> animations_;

    std::vector<animation::Stage> animation_stages_;
};

}  // namespace scene

#endif
