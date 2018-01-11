#pragma once

#include "surface_integrator.hpp"
#include "sampler/sampler_random.hpp"

namespace rendering::integrator::surface {

class alignas(64) Debug : public Integrator {

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

	Debug(rnd::Generator& rng, const take::Settings& take_settings, const Settings& settings);

	virtual void prepare(const Scene& scene, uint32_t num_samples_per_pixel) override final;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) override final;

	virtual float3 li(Ray& ray, Intersection& intersection, Worker& worker) override final;

	virtual size_t num_bytes() const override final;

private:

	Settings settings_;

	sampler::Random sampler_;
};

class Debug_factory : public Factory {

public:

	Debug_factory(const take::Settings& take_settings, uint32_t num_integrators,
				  Debug::Settings::Vector vector);

	~Debug_factory();

	virtual Integrator* create(uint32_t id, rnd::Generator& rng) const override final;

private:

	Debug* integrators_;

	Debug::Settings settings_;
};

}
