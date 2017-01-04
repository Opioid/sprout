#pragma once

#include "sub_integrator.hpp"
#include "sampler/sampler_random.hpp"

namespace scene {

class Prop;

namespace light { class Light; }

}

namespace rendering { namespace integrator { namespace surface { namespace sub {

class Bruteforce : public Integrator {

public:

	Bruteforce(const take::Settings& settings, rnd::Generator& rng);

	virtual void prepare(const Scene& scene, uint32_t num_samples_per_pixel) final override;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) final override;

	virtual float3 li(Worker& worker, const Ray& ray,
					  const Intersection& intersection) final override;

	virtual size_t num_bytes() const final override;

private:

	float3 transmittance(Worker& worker, Ray& ray, const scene::Prop* prop,
						 const scene::light::Light& light) const;

	sampler::Random sampler_;
};

}}}}
