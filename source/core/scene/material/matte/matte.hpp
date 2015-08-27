#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample_cache.hpp"
#include "scene/material/material_sample.hpp"
#include "scene/material/lambert/lambert.hpp"
//#include "scene/material/oren_nayar/oren_nayar.hpp"

namespace scene { namespace material { namespace matte {

class Sample : public material::Sample {
public:

	Sample();

	virtual math::float3 evaluate(const math::float3& wi, float& pdf) const final override;

	virtual math::float3 emission() const final override;

	virtual math::float3 attenuation() const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler, BxDF_result& result) const final override;

	virtual bool is_pure_emissive() const final override;

	virtual bool is_translucent() const final override;

	void set(const math::float3& color, float sqrt_roughness);

private:

	math::float3 diffuse_color_;

	float a2_;

	lambert::Lambert<Sample> lambert_;
//	oren_nayar::Oren_nayar<Sample> oren_nayar_;

	friend lambert::Lambert<Sample>;
//	friend oren_nayar::Oren_nayar<Sample>;
};

class Matte : public Material<Generic_sample_cache<Sample>> {
public:

	Matte(Generic_sample_cache<Sample>& cache, std::shared_ptr<image::texture::Texture_2D> mask);

	virtual math::float3 sample_emission(math::float2 uv,
										 const image::texture::sampler::Sampler_2D& sampler) const override;

	virtual math::float3 average_emission() const override;

	virtual const image::texture::Texture_2D* emission_map() const final override;
};

}}}

