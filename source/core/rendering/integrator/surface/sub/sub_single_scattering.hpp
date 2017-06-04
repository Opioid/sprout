#pragma once

#include "sub_integrator.hpp"
#include "sampler/sampler_random.hpp"

namespace scene {

namespace material { class BSSRDF; }

namespace light { class Light; }

class Prop;

}

namespace rendering { namespace integrator { namespace surface { namespace sub {

class alignas(64) Single_scattering : public Integrator {

public:

	struct Settings {
		float step_size;
	};

	Single_scattering(rnd::Generator& rng, const take::Settings& take_settings,
					  const Settings& settings);

	virtual void prepare(const Scene& scene, uint32_t num_samples_per_pixel) final override;

	virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) final override;

	virtual float3 li(Worker& worker, const Ray& ray,
					  const Intersection& intersection) final override;

	virtual float3 li(Worker& worker, Ray& ray, Intersection& intersection,
					  sampler::Sampler& sampler, Sampler_filter filter,
					  Bxdf_result& sample_result) final override;

	virtual size_t num_bytes() const final override;

private:

	const Settings settings_;

	sampler::Random sampler_;
};

class Single_scattering_factory : public Factory {

public:

	Single_scattering_factory(const take::Settings& take_settings, uint32_t num_integrators,
							  float step_size);

	~Single_scattering_factory();

	virtual Integrator* create(uint32_t id, rnd::Generator& rng) const final override;

private:

	Single_scattering* integrators_;

	Single_scattering::Settings settings_;
};

}}}}
