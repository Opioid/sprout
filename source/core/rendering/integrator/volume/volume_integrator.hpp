#pragma once

#include "rendering/integrator/integrator.hpp"
#include "scene/material/sampler_settings.hpp"
#include "base/math/vector.hpp"

namespace scene {

struct Ray;

namespace volume { class Volume; }

}

namespace rendering {

class Worker;

namespace integrator { namespace volume {

class Integrator : public integrator::Integrator {

public:

	using Sampler_filter = scene::material::Sampler_settings::Filter;

	Integrator(uint32_t num_samples_per_pixel,
			   const take::Settings& settings,
			   random::Generator& rng);

	virtual ~Integrator();

	virtual float3 transmittance(Worker& worker, const scene::volume::Volume& volume,
								 const scene::Ray& ray) = 0;

	virtual float4 li(Worker& worker, const scene::volume::Volume& volume,
					  const scene::Ray& ray, float3& transmittance) = 0;
};

class Factory {

public:

	Factory(const take::Settings& settings);

	virtual Integrator* create(uint32_t num_samples_per_pixel,
							   random::Generator& rng) const = 0;

protected:

	const take::Settings& take_settings_;
};

}}}
