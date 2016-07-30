#pragma once

#include "base/math/random/generator.hpp"

namespace take { struct Settings; }

namespace rendering { namespace integrator {

enum class Light_sampling_strategy {
	One,
	All
};

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
