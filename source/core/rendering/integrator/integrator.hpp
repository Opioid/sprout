#ifndef SU_RENDERING_INTEGRATOR_INTEGRATOR_HPP
#define SU_RENDERING_INTEGRATOR_INTEGRATOR_HPP

#include "scene/material/sampler_settings.hpp"
#include "take/take_settings.hpp"
#include <cstddef>
#include <cstdint>

namespace rnd { class Generator; }

namespace scene {

namespace entity { struct Composed_transformation; }

namespace material {

namespace bxdf { struct Sample; enum class Type; }

class Material;
class Sample;

}

namespace light { class Light; }

namespace prop { class Prop; struct Intersection; }

struct Ray;
class Scene;

}

namespace rendering::integrator {

struct Light_sampling {
	enum class Strategy {
		Single,
		All
	};

	Strategy strategy;
	uint32_t num_samples;
};

class Integrator {

public:

	using Ray			  = scene::Ray;
	using Scene			  = scene::Scene;
	using Transformation  = scene::entity::Composed_transformation;
	using Light			  = scene::light::Light;
	using Material		  = scene::material::Material;
	using Material_sample = scene::material::Sample;
	using Sampler_filter  = scene::material::Sampler_settings::Filter;
	using Bxdf_sample	  = scene::material::bxdf::Sample;
	using Bxdf_type		  = scene::material::bxdf::Type;
	using Prop			  = scene::prop::Prop;
	using Intersection	  = scene::prop::Intersection;

	Integrator(rnd::Generator& rng, take::Settings const& settings);
	virtual ~Integrator();

	virtual void prepare(Scene const& scene, uint32_t num_samples_per_pixel) = 0;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) = 0;

	virtual size_t num_bytes() const = 0;

protected:

	rnd::Generator& rng_;
	const take::Settings take_settings_;
};

}

#endif
