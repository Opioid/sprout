#pragma once

#include "rendering/integrator/integrator.hpp"

namespace sampler { class Sampler; }

namespace scene {

struct Ray;

namespace material { namespace bxdf { struct Result; } }

}

namespace rendering { namespace integrator { namespace surface { namespace transmittance {

class Open : public integrator::Integrator {
public:

	Open(const take::Settings& take_settings, math::random::Generator& rng);

	math::float3 resolve(Worker& worker, scene::Ray& ray, scene::Intersection& intersection,
						 const math::float3& attenuation,
						 sampler::Sampler& sampler,
						 const image::texture::sampler::Sampler_2D& texture_sampler,
						 scene::material::bxdf::Result& sample_result);
};

}}}}
