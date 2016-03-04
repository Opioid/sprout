#pragma once

#include "rendering/integrator/integrator.hpp"
#include "scene/material/texture_filter.hpp"

namespace sampler { class Sampler; }

namespace scene {

struct Ray;

namespace material { namespace bxdf { struct Result; } }

}

namespace rendering { namespace integrator { namespace surface { namespace transmittance {

class Open : public integrator::Integrator {
public:

	Open(const take::Settings& take_settings, math::random::Generator& rng, uint32_t max_bounces);

	math::float3 resolve(Worker& worker, scene::Ray& ray, scene::Intersection& intersection,
						 const math::float3& attenuation,
						 sampler::Sampler& sampler,
						 scene::material::Texture_filter override_filter,
						 scene::material::bxdf::Result& sample_result);

private:

	uint32_t max_bounces_;
};

}}}}
