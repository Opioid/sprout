#pragma once

#include "sub_integrator.hpp"

namespace rendering { namespace integrator { namespace surface { namespace sub {

class Pathtracer : public Integrator {

public:

	Pathtracer(const take::Settings& settings, rnd::Generator& rng);

	virtual void prepare(const Scene& scene, uint32_t num_samples_per_pixel) final override;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) final override;

	virtual float3 li(Worker& worker, const Ray& ray,
					  const Intersection& intersection) final override;

	virtual size_t num_bytes() const final override;
};

}}}}
