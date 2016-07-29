#pragma once

#include "rendering/integrator/integrator.hpp"
#include "scene/material/sampler_settings.hpp"
#include "base/math/vector.hpp"

namespace scene {

namespace material { namespace bxdf { struct Result; enum class Type; } }

struct Intersection;
struct Ray;

}

namespace rendering {

class Worker;

namespace integrator { namespace surface {

enum class Light_sampling_strategy {
	Sample_one,
	Sample_all
};

class Integrator : public integrator::Integrator {

public:

	Integrator(const take::Settings& settings, math::random::Generator& rng);
	virtual ~Integrator();

	virtual float4 li(Worker& worker, scene::Ray& ray, bool volume,
					  scene::Intersection& intersection) = 0;

protected:

	using Sampler_filter = scene::material::Sampler_settings::Filter;
	using Bxdf_result    = scene::material::bxdf::Result;
	using Bxdf_type		 = scene::material::bxdf::Type;

	bool resolve_mask(Worker& worker, scene::Ray& ray,
					  scene::Intersection& intersection,
					  Sampler_filter filter);
};

class Integrator_factory {

public:

	Integrator_factory(const take::Settings& settings);

	virtual Integrator* create(math::random::Generator& rng) const = 0;

protected:

	const take::Settings& take_settings_;
};

}}}
