#pragma once

#include "base/math/vector.hpp"

namespace random { class Generator; }

namespace take { struct Settings; }

namespace rendering { namespace integrator {

struct Light_sampling {
	enum class Strategy {
		One,
		All
	};

	Strategy strategy;
	uint32_t num_samples;
};

class Integrator {

public:

	Integrator(uint32_t samples_per_pixel,
			   const take::Settings& settings,
			   random::Generator& rng);

	virtual ~Integrator();

	virtual void resume_pixel(uint32_t sample, uint2 seed) = 0;

	const take::Settings& take_settings() const;

protected:

	const take::Settings& take_settings_;
	random::Generator& rng_;
};

}}
