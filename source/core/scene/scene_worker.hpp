#ifndef SU_CORE_SCENE_WORKER_HPP
#define SU_CORE_SCENE_WORKER_HPP

#include "base/math/vector.hpp"
#include "base/random/generator.hpp"
#include "material/material_sample_cache.hpp"
#include "material/sampler_cache.hpp"
#include "material/material_sample.hpp"
#include "prop/interface_stack.hpp"
#include "shape/node_stack.hpp"
#include "take/take_settings.hpp"

namespace scene {

class Scene;
struct Ray;

namespace material {
class Sample;
}

namespace prop {
class Prop;
struct Intersection;
}  // namespace prop

class Worker {
  public:
    using Sampler_filter     = material::Sampler_settings::Filter;
    using Texture_sampler_2D = image::texture::sampler::Sampler_2D;
    using Texture_sampler_3D = image::texture::sampler::Sampler_3D;
    using Intersection       = prop::Intersection;
    using Interface_stack    = prop::Interface_stack;

    Worker() noexcept;

    ~Worker() noexcept;

    void init(uint32_t id, take::Settings const& settings, Scene const& scene,
              uint32_t max_material_sample_size, uint32_t max_material_sample_depth) noexcept;

    uint32_t id() const noexcept;

    bool intersect(Ray& ray, Intersection& intersection) const noexcept;
    bool intersect(Ray& ray, float& epsilon) const noexcept;

    bool resolve_mask(Ray& ray, Intersection& intersection, Sampler_filter filter) noexcept;

    bool intersect_and_resolve_mask(Ray& ray, Intersection& intersection,
                                    Sampler_filter filter) noexcept;

    bool visibility(Ray const& ray) const noexcept;

    bool masked_visibility(Ray const& ray, Sampler_filter filter, float& mv) const noexcept;

    Scene const& scene() const noexcept;

    shape::Node_stack& node_stack() const noexcept;

    material::Sample_cache& sample_cache() const noexcept;

    template <typename T>
    T& sample(uint32_t depth) const noexcept;

    Texture_sampler_2D const& sampler_2D(uint32_t key, Sampler_filter filter) const noexcept;

    Texture_sampler_3D const& sampler_3D(uint32_t key, Sampler_filter filter) const noexcept;

    Interface_stack& interface_stack() noexcept;

    void reset_interface_stack(Interface_stack const& stack) noexcept;

    float ior_outside(float3 const& wo, Intersection const& intersection) const noexcept;

    void interface_change(float3 const& dir, Intersection const& intersection) noexcept;

    material::IoR interface_change_ior(float3 const& dir, Intersection const& intersection) noexcept;

  private:
    uint32_t id_;

  protected:
    rnd::Generator rng_;

    take::Settings settings_;

    Scene const* scene_;

    //   Tile_queue const* tiles_;

    mutable shape::Node_stack node_stack_;

    mutable material::Sample_cache sample_cache_;

    material::Sampler_cache const sampler_cache_;

    Interface_stack interface_stack_;
    Interface_stack interface_stack_temp_;
};

}  // namespace scene

#endif
