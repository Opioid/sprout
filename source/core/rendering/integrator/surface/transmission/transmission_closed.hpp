#pragma once

#include "rendering/integrator/integrator.hpp"

namespace sampler { class Sampler; }

namespace scene { namespace material { namespace bxdf { struct Result; } } }

namespace rendering { namespace integrator { namespace surface { namespace transmission {

class Closed : public integrator::Integrator {
public:

	Closed(const take::Settings& take_settings, math::random::Generator& rng);

	math::float3 resolve(Worker& worker, math::Oray& ray, scene::Intersection& intersection,
						 const math::float3& attenuation,
						 sampler::Sampler& sampler,
						 const image::texture::sampler::Sampler_2D& texture_sampler,
						 scene::material::bxdf::Result& sample_result);
};

}}}}
