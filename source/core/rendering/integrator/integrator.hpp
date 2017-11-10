#ifndef SU_RENDERING_INTEGRATOR_INTEGRATOR_HPP
#define SU_RENDERING_INTEGRATOR_INTEGRATOR_HPP

#include "scene/material/sampler_settings.hpp"
#include "take/take_settings.hpp"
#include <cstddef>
#include <cstdint>

namespace rnd { class Generator; }

namespace scene {

namespace material {

namespace bxdf { struct Sample; enum class Type; }

class Sample;

}

namespace light { class Light; }

struct Intersection;
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

	using Intersection	= scene::Intersection;
	using Ray			= scene::Ray;
	using Scene			= scene::Scene;
	using Light			= scene::light::Light;

	using Material_sample = scene::material::Sample;
	using Sampler_filter  = scene::material::Sampler_settings::Filter;
	using Bxdf_sample	  = scene::material::bxdf::Sample;
	using Bxdf_type		  = scene::material::bxdf::Type;

	Integrator(rnd::Generator& rng, const take::Settings& settings);
	virtual ~Integrator();

	virtual void prepare(const Scene& scene, uint32_t num_samples_per_pixel) = 0;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) = 0;

	virtual size_t num_bytes() const = 0;

protected:

	rnd::Generator& rng_;
	const take::Settings take_settings_;
};

}

#endif
