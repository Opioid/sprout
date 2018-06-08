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
  using Node_stack = shape::Node_stack;
  using Sampler_filter = material::Sampler_settings::Filter;
  using Shape_ptr = std::shared_ptr<shape::Shape>;
  using Entity = entity::Entity;
  using Prop = prop::Prop;

  Scene(take::Settings const& settings);
  ~Scene();

  void finish();

  math::AABB const& aabb() const;

  bool intersect(Ray& ray, Node_stack& node_stack, prop::Intersection& intersection) const;

  bool intersect(Ray& ray, Node_stack& node_stack, float& epsilon) const;

  bool intersect_volume(Ray& ray, Node_stack& node_stack, prop::Intersection& intersection) const;

  bool intersect_p(Ray const& ray, Node_stack& node_stack) const;

  float opacity(Ray const& ray, Sampler_filter filter, Worker const& worker) const;

  float3 thin_absorption(Ray const& ray, Sampler_filter filter, Worker const& worker) const;

  float tick_duration() const;
  float simulation_time() const;
  uint64_t current_tick() const;

  Entity* entity(size_t index) const;
  Entity* entity(std::string_view name) const;

  std::vector<light::Light*> const& lights() const;

  struct Light {
    const light::Light& ref;
    float pdf;
  };
  Light light(uint32_t id) const;
  Light random_light(float random) const;

  bool has_volumes() const;

  void tick(thread::Pool& thread_pool);
  float seek(float time, thread::Pool& thread_pool);

  void compile(thread::Pool& pool);

  entity::Dummy* create_dummy();
  entity::Dummy* create_dummy(std::string const& name);

  Prop* create_prop(Shape_ptr const& shape, Materials const& materials);

  Prop* create_prop(Shape_ptr const& shape, Materials const& materials, std::string const& name);

  light::Prop_light* create_prop_light(Prop* prop, uint32_t part);
  light::Prop_image_light* create_prop_image_light(Prop* prop, uint32_t part);

  void add_extension(Entity* extension);
  void add_extension(Entity* extension, std::string const& name);

  void add_material(Material_ptr const& material);
  void add_animation(std::shared_ptr<animation::Animation> const& animation);

  void create_animation_stage(Entity* entity, animation::Animation* animation);

  size_t num_bytes() const;

 private:
  void add_named_entity(Entity* entity, std::string const& name);

  const take::Settings take_settings_;

  double tick_duration_ = 1.0 / 60.0;
  double simulation_time_ = 0.0;
  uint64_t current_tick_ = 0;

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
