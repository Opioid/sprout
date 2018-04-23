#ifndef SU_CORE_RENDERING_WORKER_HPP
#define SU_CORE_RENDERING_WORKER_HPP

#include "scene/scene_worker.hpp"
#include "scene/prop/interface_stack.hpp"

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

	float4 li(Ray& ray, const scene::prop::Interface_stack& interface_stack);

	using Material_sample = scene::material::Sample;

	bool volume(Ray& ray, Intersection& intersection, Sampler_filter filter,
				float3& li, float3& transmittance, float3& weight);

	float3 transmittance(const Ray& ray);

	float3 tinted_visibility(const Ray& ray, Sampler_filter filter);

	float3 tinted_visibility(Ray& ray, const Intersection& intersection, Sampler_filter filter);

	sampler::Sampler* sampler();

	scene::prop::Interface_stack& interface_stack();

	void interface_change(const float3& dir, const Intersection& intersection);

protected:

	integrator::surface::Integrator* surface_integrator_ = nullptr;
	integrator::volume::Integrator*  volume_integrator_  = nullptr;
	sampler::Sampler* sampler_ = nullptr;

	scene::prop::Interface_stack interface_stack_;
	scene::prop::Interface_stack interface_stack_temp_;
};

}

#endif
