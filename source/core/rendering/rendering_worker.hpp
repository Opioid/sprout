#ifndef SU_CORE_RENDERING_WORKER_HPP
#define SU_CORE_RENDERING_WORKER_HPP

#include "scene/prop/interface_stack.hpp"
#include "scene/scene_worker.hpp"

namespace sampler {

class Sampler;
class Factory;

}  // namespace sampler

namespace scene::material {
class Sample;
}

namespace rendering {

namespace integrator {

namespace photon {
class Map;
class Mapper;
}  // namespace photon

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
    using Ray = scene::Ray;

    ~Worker();

    void init(uint32_t id, take::Settings const& settings, scene::Scene const& scene,
              uint32_t max_material_sample_size, uint32_t num_samples_per_pixel,
              integrator::surface::Factory& surface_integrator_factory,
              integrator::volume::Factory&  volume_integrator_factory,
              sampler::Factory& sampler_factory, integrator::photon::Map* photon_map,
              take::Photon_settings const& photon_settings_);

    float4 li(Ray& ray, const scene::prop::Interface_stack& interface_stack);

    using Material_sample = scene::material::Sample;

    bool volume(Ray& ray, Intersection& intersection, Sampler_filter filter, float3& li,
                float3& transmittance);

    bool transmitted_visibility(Ray& ray, Intersection const& intersection, Sampler_filter filter,
                                float3& tv);

    sampler::Sampler* sampler();

    scene::prop::Interface_stack& interface_stack();

    void interface_change(f_float3 dir, Intersection const& intersection);

    uint32_t bake_photons(int32_t begin, int32_t end, float normalized_tick_offset,
                          float normalized_tick_slice);

    float3 photon_li(f_float3 position, bool is_volumetric, Material_sample const& sample) const;

    size_t num_bytes() const;

  protected:
    bool transmittance(Ray const& ray, float3& transmittance);

    bool tinted_visibility(Ray& ray, Intersection const& intersection, Sampler_filter filter,
                           float3& tv);

    integrator::surface::Integrator* surface_integrator_ = nullptr;
    integrator::volume::Integrator*  volume_integrator_  = nullptr;

    sampler::Sampler* sampler_ = nullptr;

    integrator::photon::Mapper* photon_mapper_ = nullptr;
    integrator::photon::Map*    photon_map_    = nullptr;

    scene::prop::Interface_stack interface_stack_;
    scene::prop::Interface_stack interface_stack_temp_;
};

}  // namespace rendering

#endif
