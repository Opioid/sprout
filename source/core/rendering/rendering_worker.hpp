#ifndef SU_CORE_RENDERING_WORKER_HPP
#define SU_CORE_RENDERING_WORKER_HPP

#include "scene/scene_worker.hpp"

namespace sampler {

class Sampler;
class Pool;

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
class Lighttracer_pool;
class Importance_cache;

namespace photon {

class Map;
class Mapper;

}  // namespace photon
}  // namespace particle

namespace surface {

class Integrator;
class Pool;

}  // namespace surface

namespace volume {

class Integrator;
class Pool;

}  // namespace volume
}  // namespace integrator

class Worker : public scene::Worker {
  public:
    using Ray                 = scene::Ray;
    using Scene               = scene::Scene;
    using Surface_pool        = integrator::surface::Pool;
    using Volume_pool         = integrator::volume::Pool;
    using Lighttracer_pool    = integrator::particle::Lighttracer_pool;
    using Photon_map          = integrator::particle::photon::Map;
    using Photon_mapper       = integrator::particle::photon::Mapper;
    using Particle_importance = integrator::particle::Importance_cache;

    Worker();

    ~Worker();

    void init(uint32_t id, Scene const& scene, Camera const& camera, uint32_t num_samples_per_pixel,
              Surface_pool* surfaces, Volume_pool& volumes, sampler::Pool& samplers,
              Photon_map* photon_map, take::Photon_settings const& photon_settings_,
              Lighttracer_pool* lighttracers, uint32_t num_particles_per_chunk,
              Particle_importance* particle_importance);

    void render(uint32_t frame, uint32_t view, uint32_t iteration, int4 const& tile,
                uint32_t num_samples);

    void particles(uint32_t frame, uint64_t offset, ulong2 const& range);

    float4 li(Ray& ray, Interface_stack const& interface_stack);

    void particle_li(uint32_t frame, Interface_stack const& interface_stack);

    Event volume(Ray& ray, Intersection& intersection, Filter filter, float3& li, float3& tr);

    bool transmitted(Ray& ray, float3 const& wo, Intersection const& intersection, Filter filter,
                     float3& tr);

    uint32_t bake_photons(int32_t begin, int32_t end, uint32_t frame, uint32_t iteration);

    float3 photon_li(Intersection const& intersection, Material_sample const& sample) const;

    Particle_importance& particle_importance() const;

  protected:
    bool transmittance(Ray const& ray, float3& transmittance);

    bool tinted_visibility(Ray& ray, float3 const& wo, Intersection const& intersection,
                           Filter filter, float3& tv);

    integrator::surface::Integrator* surface_integrator_ = nullptr;

    integrator::volume::Integrator* volume_integrator_ = nullptr;

    sampler::Sampler* sampler_ = nullptr;

    Photon_mapper* photon_mapper_ = nullptr;

    Photon_map* photon_map_ = nullptr;

    integrator::particle::Lighttracer* lighttracer_ = nullptr;

    Particle_importance* particle_importance_ = nullptr;
};

}  // namespace rendering

#endif
