#pragma once

#include "base/math/vector.hpp"
#include "base/math/random/generator.hpp"

namespace image { namespace texture { namespace sampler {

class Sampler_2D;

}}}

namespace take { struct Settings; }

namespace scene { struct Intersection; }

namespace rendering {

class Worker;

namespace integrator {

class Integrator {

public:

	Integrator(const take::Settings& settings, math::random::Generator& rng);
	virtual ~Integrator();

	virtual void start_new_pixel(uint32_t num_samples);

	const take::Settings& take_settings() const;

protected:

	const take::Settings& take_settings_;
	math::random::Generator& rng_;
};

}}
