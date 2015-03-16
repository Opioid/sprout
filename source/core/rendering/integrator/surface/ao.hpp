#pragma once

#include "rendering/integrator/integrator.hpp"

namespace rendering {

class Ao : public Surface_integrator {
public:

	struct Settings {
		uint32_t num_samples;
		float num_samples_reciprocal;
		float radius;
	};

	Ao(uint32_t id,  const math::random::Generator& rng, const Settings& settings);

	virtual math::float3 li(const Worker& worker, uint32_t subsample, math::Oray& ray, scene::Intersection& intersection);

private:

	Settings settings_;
};

class Ao_factory : public Surface_integrator_factory {
public:

	Ao_factory(uint32_t num_samples, float radius);

	virtual Surface_integrator* create(uint32_t id,  const math::random::Generator& rng) const;

private:

	Ao::Settings settings_;
};

}
