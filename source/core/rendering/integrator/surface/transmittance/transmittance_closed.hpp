#pragma once

#include "rendering/integrator/integrator.hpp"
#include "scene/material/texture_filter.hpp"

namespace sampler { class Sampler; }

namespace scene {

struct Ray;

namespace material { namespace bxdf { struct Result; } }

}

namespace rendering { namespace integrator { namespace surface { namespace transmittance {

class Closed : public integrator::Integrator {
public:

	Closed(const take::Settings& take_settings, math::random::Generator& rng);

	math::float3 resolve(Worker& worker, scene::Ray& ray, scene::Intersection& intersection,
						 const math::float3& attenuation,
						 sampler::Sampler& sampler,
						 scene::material::Texture_filter override_filter,
						 scene::material::bxdf::Result& sample_result);
};

}}}}
