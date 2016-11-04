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

class Open : public integrator::Integrator {

public:

	Open(const take::Settings& take_settings, math::random::Generator& rng,
		 uint32_t max_bounces);

	using Sampler_filter = scene::material::Sampler_settings::Filter;
	using Bxdf_result    = scene::material::bxdf::Result;

	float3 resolve(Worker& worker, scene::Ray& ray, scene::Intersection& intersection,
				   float3_p attenuation, sampler::Sampler& sampler,
				   Sampler_filter filter, Bxdf_result& sample_result);

private:

	uint32_t max_bounces_;
};

}}}}
