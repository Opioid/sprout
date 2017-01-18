#pragma once

#include "rendering/integrator/surface/surface_integrator.hpp"
#include "image/texture/sampler/sampler_2d_nearest.hpp"
#include "image/texture/sampler/address_mode.hpp"

namespace rendering { namespace integrator { namespace surface {

class Debug : public Integrator {

public:

	struct Settings {
		enum class Vector {
			Tangent,
			Bitangent,
			Geometric_normal,
			Shading_normal,
			UV
		};

		Vector vector;
	};

	Debug(const take::Settings& take_settings, rnd::Generator& rng, const Settings& settings);

	virtual void prepare(const Scene& scene, uint32_t num_samples_per_pixel) final override;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) final override;

	virtual float4 li(Worker& worker, Ray& ray, Intersection& intersection) final override;

	virtual size_t num_bytes() const final override;

private:

	Settings settings_;
};

class Debug_factory : public Factory {

public:

	Debug_factory(const take::Settings& take_settings, Debug::Settings::Vector vector);

	virtual Integrator* create(rnd::Generator& rng) const final override;

private:

	Debug::Settings settings_;
};

}}}
