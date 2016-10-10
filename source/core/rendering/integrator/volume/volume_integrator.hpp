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

	Integrator(const take::Settings& settings, math::random::Generator& rng);
	virtual ~Integrator();

	virtual float3 transmittance(Worker& worker, const scene::volume::Volume* volume,
								 const scene::Ray& ray) = 0;

	virtual float4 li(Worker& worker, const scene::volume::Volume* volume,
					  const scene::Ray& ray, float3& transmittance) = 0;
};

class Factory {

public:

	Factory(const take::Settings& settings);

	virtual Integrator* create(math::random::Generator& rng) const = 0;

protected:

	const take::Settings& take_settings_;
};

}}}
