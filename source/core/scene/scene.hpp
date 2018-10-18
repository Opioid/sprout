#ifndef SU_CORE_SCENE_SCENE_HPP
#define SU_CORE_SCENE_SCENE_HPP

#include <map>
#include <memory>
#include <vector>
#include "base/math/distribution/distribution_1d.hpp"
#include "bvh/scene_bvh_builder.hpp"
#include "light/null_light.hpp"
#include "material/material.hpp"
#include "prop/prop_bvh_wrapper.hpp"
#include "scene_constants.hpp"
#include "take/take_settings.hpp"

namespace thread {
class Pool;
}

namespace scene {

namespace shape {
class Shape;
}

namespace animation {

class Animation;
class Stage;

}  // namespace animation

namespace entity {

class Entity;
class Dummy;

}  // namespace entity

namespace light {

class Light;
class Prop_light;
class Prop_image_light;

}  // namespace light

namespace prop {

struct Intersection;
class Prop;

}  // namespace prop

class Worker;

struct Ray;

class Scene {
  public:
    using Node_stack     = shape::Node_stack;
    using Sampler_filter = material::Sampler_settings::Filter;
    using Shape_ptr      = std::shared_ptr<shape::Shape>;
    using Entity         = entity::Entity;
    using Prop           = prop::Prop;

    Scene(take::Settings const& settings) noexcept;

    ~Scene() noexcept;

    void finish(uint64_t frame_step, uint64_t frame_duration) noexcept;

    math::AABB const& aabb() const noexcept;

    math::AABB caustic_aabb() const noexcept;

    bool is_infinite() const noexcept;

    bool has_volumes() const noexcept;

    bool intersect(Ray& ray, Node_stack& node_stack, prop::Intersection& intersection) const
        noexcept;

    bool intersect(Ray& ray, Node_stack& node_stack, float& epsilon) const noexcept;

    bool intersect_volume(Ray& ray, Node_stack& node_stack, prop::Intersection& intersection) const
        noexcept;

    bool intersect_p(Ray const& ray, Node_stack& node_stack) const noexcept;

    bool opacity(Ray const& ray, Sampler_filter filter, Worker const& worker, float& o) const
        noexcept;

    bool thin_absorption(Ray const& ray, Sampler_filter filter, Worker const& worker,
                         float3& ta) const noexcept;

    Entity* entity(size_t index) const noexcept;
    Entity* entity(std::string_view name) const noexcept;

    std::vector<light::Light*> const& lights() const noexcept;

    struct Light {
        light::Light const& ref;
        float               pdf;
    };
    Light light(uint32_t id, bool calculate_pdf = true) const noexcept;

    Light random_light(float random) const noexcept;

    void simulate(uint64_t start, uint64_t end, thread::Pool& thread_pool) noexcept;

    void compile(uint64_t time, thread::Pool& pool) noexcept;

    entity::Dummy* create_dummy() noexcept;
    entity::Dummy* create_dummy(std::string const& name) noexcept;

    Prop* create_prop(Shape_ptr const& shape, Materials const& materials) noexcept;

    Prop* create_prop(Shape_ptr const& shape, Materials const& materials,
                      std::string const& name) noexcept;

    light::Prop_light* create_prop_light(Prop* prop, uint32_t part) noexcept;

    light::Prop_image_light* create_prop_image_light(Prop* prop, uint32_t part) noexcept;

    void add_extension(Entity* extension) noexcept;
    void add_extension(Entity* extension, std::string const& name) noexcept;

    void add_material(Material_ptr const& material) noexcept;

    void add_animation(std::shared_ptr<animation::Animation> const& animation) noexcept;

    void create_animation_stage(Entity* entity, animation::Animation* animation) noexcept;

    size_t num_bytes() const noexcept;

  private:
    void add_named_entity(Entity* entity, std::string const& name) noexcept;

    uint32_t count_frames(uint64_t frame_step, uint64_t frame_duration) const;

    take::Settings const take_settings_;

    uint64_t tick_duration_ = Units_per_second / 60;

    bvh::Builder<prop::Prop> bvh_builder_;

    prop::BVH_wrapper prop_bvh_;
    prop::BVH_wrapper volume_bvh_;

    light::Null_light null_light_;

    bool has_masked_material_;
    bool has_tinted_shadow_;
    bool has_volumes_;

    std::vector<entity::Dummy*> dummies_;

    std::vector<prop::Prop*> finite_props_;
    std::vector<prop::Prop*> infinite_props_;

    std::vector<prop::Prop*> volumes_;
    std::vector<prop::Prop*> infinite_volumes_;

    std::vector<light::Light*> lights_;

    std::vector<entity::Entity*> extensions_;

    std::vector<entity::Entity*> entities_;

    std::map<std::string, entity::Entity*, std::less<>> named_entities_;

    std::vector<float> light_powers_;

    math::Distribution_1D light_distribution_;

    std::vector<Material_ptr> materials_;

    std::vector<std::shared_ptr<animation::Animation>> animations_;

    std::vector<animation::Stage> animation_stages_;
};

}  // namespace scene

#endif
