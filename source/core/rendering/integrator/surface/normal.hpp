#pragma once

#include "rendering/integrator/integrator.hpp"
#include "image/texture/sampler/sampler_2d_nearest.hpp"
#include "image/texture/sampler/address_mode.hpp"

namespace rendering {

class Normal : public Surface_integrator {
public:

	struct Settings {
		enum class Vector {
			Tangent,
			Bitangent,
			Geometric_normal,
			Shading_normal
		};

		Vector vector;

		image::texture::sampler::Sampler_2D_nearest<image::texture::sampler::Address_mode_repeat> sampler;
	};

	Normal(const take::Settings& take_settings, math::random::Generator& rng, const Settings& settings);

	virtual void start_new_pixel(uint32_t num_samples) final override;

	virtual math::float4 li(Worker& worker, math::Oray& ray, scene::Intersection& intersection) final override;

private:

	Settings settings_;
};

class Normal_factory : public Surface_integrator_factory {
public:

	Normal_factory(const take::Settings& take_settings, Normal::Settings::Vector vector);

	virtual Surface_integrator* create(math::random::Generator& rng) const final override;

private:

	Normal::Settings settings_;
};

}
