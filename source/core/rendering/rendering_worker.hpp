#ifndef SU_CORE_RENDERING_WORKER_HPP
#define SU_CORE_RENDERING_WORKER_HPP

#include "scene/scene_worker.hpp"

namespace sampler {

class Sampler;
class Factory;

}

namespace scene::material { class Sample; }

namespace rendering {

namespace integrator {

namespace surface {

class Integrator;
class Factory;

}

namespace volume {

class Integrator;
class Factory;

}}

class Worker : public scene::Worker {

public:

	using Ray = scene::Ray;

	~Worker();

	void init(uint32_t id, const take::Settings& settings,
			  const scene::Scene& scene, uint32_t max_sample_size,
			  integrator::surface::Factory& surface_integrator_factory,
			  integrator::volume::Factory& volume_integrator_factory,
			  sampler::Factory& sampler_factory);

	void prepare(uint32_t num_samples_per_pixel);

	float4 li(Ray& ray);
	float3 li(Ray& ray, Intersection& intersection);
	float3 volume_li(const Ray& ray, float3& transmittance);

	using Material_sample = scene::material::Sample;

	bool volume(Ray& ray, Intersection& intersection, Sampler_filter filter,
				float3& li, float3& transmittance, float3& weight);

	float3 transmittance(const Ray& ray) const;

	float3 tinted_visibility(const Ray& ray, Sampler_filter filter) const;

	float3 tinted_visibility(Ray& ray, const Intersection& intersection, Sampler_filter filter);

	sampler::Sampler* sampler();

protected:

	integrator::surface::Integrator* surface_integrator_ = nullptr;
	integrator::volume::Integrator*  volume_integrator_  = nullptr;
	sampler::Sampler* sampler_ = nullptr;
};

}

#endif
