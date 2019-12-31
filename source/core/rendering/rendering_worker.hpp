#ifndef SU_CORE_RENDERING_WORKER_HPP
#define SU_CORE_RENDERING_WORKER_HPP

#include "scene/scene_worker.hpp"

namespace sampler {

class Sampler;
class Factory;

}  // namespace sampler

namespace take {
struct Photon_settings;
}

namespace scene {

namespace material {
class Sample;
}

}  // namespace scene

namespace rendering {

enum class Event;

namespace integrator {

namespace particle {

class Lighttracer;
class Lighttracer_factory;
class Importance_cache;

namespace photon {

class Map;
class Mapper;

}  // namespace photon
}  // namespace particle

namespace surface {

class Integrator;
class Factory;

}  // namespace surface

namespace volume {

class Integrator;
class Factory;

}  // namespace volume
}  // namespace integrator

class Worker : public scene::Worker {
  public:
    using Ray                 = scene::Ray;
    using Scene               = scene::Scene;
    using Material_sample     = scene::material::Sample;
    using Surface_factory     = integrator::surface::Factory;
    using Volume_factory      = integrator::volume::Factory;
    using Lighttracer_factory = integrator::particle::Lighttracer_factory;
    using Photon_map          = integrator::particle::photon::Map;
    using Photon_mapper       = integrator::particle::photon::Mapper;
    using Particle_importance = integrator::particle::Importance_cache;

    Worker(uint32_t max_sample_size) noexcept;

    ~Worker() noexcept;

    void init(uint32_t id, Scene const& scene, Camera const& camera, uint32_t num_samples_per_pixel,
              Surface_factory& surface_factory, Volume_factory& volume_factory,
              sampler::Factory& sampler_factory, Photon_map* photon_map,
              take::Photon_settings const& photon_settings_,
              Lighttracer_factory* lighttracer_factory, uint32_t num_particles_per_chunk,
              Particle_importance* particle_importance) noexcept;

    float4 li(Ray& ray, Interface_stack const& interface_stack) noexcept;

    void particle_li(uint32_t frame, int4 const& bounds,
                     Interface_stack const& interface_stack) noexcept;

    Event volume(Ray& ray, Intersection& intersection, Filter filter, float3& li,
                 float3& transmittance) noexcept;

    bool transmitted(Ray& ray, float3 const& wo, Intersection const& intersection, Filter filter,
                     float3& tr) noexcept;

    uint32_t bake_photons(int32_t begin, int32_t end, uint32_t frame, uint32_t iteration) noexcept;

    float3 photon_li(Intersection const& intersection, Material_sample const& sample) const
        noexcept;

    Particle_importance& particle_importance() const noexcept;

  protected:
    bool transmittance(Ray const& ray, float3& transmittance) noexcept;

    bool tinted_visibility(Ray& ray, float3 const& wo, Intersection const& intersection,
                           Filter filter, float3& tv) noexcept;

    integrator::surface::Integrator* surface_integrator_ = nullptr;
    integrator::volume::Integrator*  volume_integrator_  = nullptr;

    sampler::Sampler* sampler_ = nullptr;

    Photon_mapper* photon_mapper_ = nullptr;
    Photon_map*    photon_map_    = nullptr;

    integrator::particle::Lighttracer* lighttracer_ = nullptr;

    Particle_importance* particle_importance_ = nullptr;
};

}  // namespace rendering

#endif
