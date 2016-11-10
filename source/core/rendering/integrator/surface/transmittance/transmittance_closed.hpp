#pragma once

#include "rendering/integrator/integrator.hpp"
#include "scene/material/sampler_settings.hpp"
#include "base/math/vector.hpp"

namespace sampler { class Sampler; }

namespace scene {

namespace material { namespace bxdf { struct Result; } }

struct Intersection;
struct Ray;

}

namespace rendering {

class Worker;

namespace integrator { namespace surface { namespace transmittance {

class Closed : public integrator::Integrator {

public:

	Closed(uint32_t num_samples_per_pixel,
		   const take::Settings& take_settings,
		   random::Generator& rng);

	virtual void resume_pixel(uint32_t sample, uint2 seed) final override;

	using Sampler_filter = scene::material::Sampler_settings::Filter;
	using Bxdf_result = scene::material::bxdf::Result;

	float3 resolve(Worker& worker, scene::Ray& ray, scene::Intersection& intersection,
				   float3_p attenuation, sampler::Sampler& sampler,
				   Sampler_filter filter, Bxdf_result& sample_result);
};

}}}}
