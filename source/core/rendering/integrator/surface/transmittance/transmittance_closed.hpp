#pragma once

#include "rendering/integrator/integrator.hpp"

namespace sampler { class Sampler; }

namespace scene {

struct Ray;

namespace material { namespace bxdf { struct Result; } }

}

namespace rendering { namespace integrator { namespace surface { namespace transmittance {

class Closed : public integrator::Integrator {
public:

	Closed(const take::Settings& take_settings, math::random::Generator& rng);

	math::vec3 resolve(Worker& worker, scene::Ray& ray, scene::Intersection& intersection,
						 const math::vec3& attenuation,
						 sampler::Sampler& sampler,
						 const image::texture::sampler::Sampler_2D& texture_sampler,
						 scene::material::bxdf::Result& sample_result);
};

}}}}
