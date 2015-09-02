#pragma once

#include "base/math/vector.hpp"
#include "base/math/ray.hpp"
#include "base/math/random/generator.hpp"

namespace image { namespace texture { namespace sampler {

class Sampler_2D;

}}}

namespace take {

struct Settings;

}

namespace scene {

struct Intersection;

}

namespace rendering {

class Worker;

class Integrator {
public:

	Integrator(const take::Settings& settings, math::random::Generator& rng);
	virtual ~Integrator();

	virtual void start_new_pixel(uint32_t num_samples);

protected:

	const take::Settings& take_settings_;
	math::random::Generator& rng_;
};

class Surface_integrator : public Integrator {
public:

	Surface_integrator(const take::Settings& settings, math::random::Generator& rng);
	virtual ~Surface_integrator();

	virtual math::float4 li(Worker& worker, math::Oray& ray, scene::Intersection& intersection) = 0;

protected:

	bool resolve_mask(Worker& worker, math::Oray& ray, scene::Intersection& intersection,
					  const image::texture::sampler::Sampler_2D& texture_sampler);
};

class Surface_integrator_factory {
public:

	Surface_integrator_factory(const take::Settings& settings);

	virtual Surface_integrator* create(math::random::Generator& rng) const = 0;

protected:

	const take::Settings& take_settings_;
};

}
