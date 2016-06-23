#pragma once

#include "rendering/integrator/integrator.hpp"
#include "scene/material/sampler_settings.hpp"

namespace sampler { class Sampler; }

namespace scene {

struct Ray;

namespace material { namespace bxdf { struct Result; } }

}

namespace rendering { namespace integrator { namespace surface { namespace transmittance {

class Closed : public integrator::Integrator {

public:

	Closed(const take::Settings& take_settings, math::random::Generator& rng);

	using Sampler_filter = scene::material::Sampler_settings::Filter;
	using Bxdf_result = scene::material::bxdf::Result;

	float3 resolve(Worker& worker, scene::Ray& ray, scene::Intersection& intersection,
						 const float3& attenuation, sampler::Sampler& sampler,
						 Sampler_filter filter, Bxdf_result& sample_result);
};

}}}}
