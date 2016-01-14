#pragma once

#include "rendering/integrator/integrator.hpp"

namespace rendering { namespace integrator { namespace surface {

class Integrator : public integrator::Integrator {
public:

	Integrator(const take::Settings& settings, math::random::Generator& rng);
	virtual ~Integrator();

	virtual math::float4 li(Worker& worker, math::Oray& ray, bool volume, scene::Intersection& intersection) = 0;

protected:

	bool resolve_mask(Worker& worker, math::Oray& ray, scene::Intersection& intersection,
					  const image::texture::sampler::Sampler_2D& texture_sampler);
};

class Integrator_factory {
public:

	Integrator_factory(const take::Settings& settings);

	virtual Integrator* create(math::random::Generator& rng) const = 0;

protected:

	const take::Settings& take_settings_;
};

}}}
