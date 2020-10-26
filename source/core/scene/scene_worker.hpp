#ifndef SU_CORE_SCENE_WORKER_HPP
#define SU_CORE_SCENE_WORKER_HPP

#include "base/math/vector.hpp"
#include "base/random/generator.hpp"
#include "material/material_sample.hpp"
#include "material/material_sample_cache.hpp"
#include "material/sampler_settings.hpp"
#include "prop/interface_stack.hpp"
#include "shape/node_stack.hpp"

namespace image::texture {
class Texture;
class Sampler_2D;
class Sampler_3D;
}  // namespace image::texture

namespace scene {

class Scene;
struct Ray;
struct Result1;

namespace camera {
class Camera;
}

namespace material {
class Sample;
}

namespace shape {
struct Normals;
}  // namespace shape

namespace prop {
class Prop;
struct Intersection;
}  // namespace prop

class Worker {
  public:
    using Camera             = camera::Camera;
    using Filter             = material::Sampler_settings::Filter;
    using Material_sample    = material::Sample;
    using Texture            = image::texture::Texture;
    using Texture_sampler_2D = image::texture::Sampler_2D;
    using Texture_sampler_3D = image::texture::Sampler_3D;
    using Intersection       = prop::Intersection;
    using Interface_stack    = prop::Interface_stack;
    using Sampler            = sampler::Sampler;

    Worker();

    ~Worker();

    void init(Scene const& scene, Camera const& camera);

    void init_rng(uint64_t sequence);

    bool intersect(Ray& ray, Intersection& isec);

    bool intersect(Ray& ray, shape::Normals& normals);

    bool resolve_mask(Ray& ray, Intersection& isec, Filter filter);

    bool intersect_and_resolve_mask(Ray& ray, Intersection& isec, Filter filter);

    Result1 visibility(Ray const& ray, Filter filter);

    Scene const& scene() const;

    Camera const& camera() const;

    uint64_t absolute_time(uint32_t frame, float frame_delta) const;

    shape::Node_stack& node_stack();

    material::Sample_cache& sample_cache();

    template <typename T>
    T& sample();

    Texture_sampler_2D const& sampler_2D(uint32_t key, Filter filter) const;

    Texture_sampler_3D const& sampler_3D(uint32_t key, Filter filter) const;

    Texture const* texture(uint32_t id) const;

    Interface_stack& interface_stack();

    RNG& rng();

    void reset_interface_stack(Interface_stack const& stack);

    float ior_outside(float3 const& wo, Intersection const& isec) const;

    void interface_change(float3 const& dir, Intersection const& isec);

    material::IoR interface_change_ior(float3 const& dir, Intersection const& isec);

    Material_sample const& sample_material(Ray const& ray, float3 const& wo, float3 const& wo1,
                                           Intersection const& isec, Filter filter, float alpha,
                                           bool avoid_caustics, bool straight_border,
                                           Sampler& sampler);

    float4 screenspace_differential(Renderstate const& rs, uint64_t time) const;

  protected:
    Scene const* scene_;

    Camera const* camera_;

    rnd::Generator rng_;

    Interface_stack interface_stack_;
    Interface_stack interface_stack_temp_;

    shape::Node_stack node_stack_;

    material::Sample_cache sample_cache_;
};

}  // namespace scene

#endif
