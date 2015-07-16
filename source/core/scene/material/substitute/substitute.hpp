#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample.hpp"
#include "scene/material/ggx/ggx.hpp"
//#include "scene/material/lambert/lambert.hpp"
#include "scene/material/oren_nayar/oren_nayar.hpp"

namespace scene { namespace material { namespace substitute {

class Sample : public material::Sample {
public:

	Sample();

	virtual math::float3 evaluate(const math::float3& wi, float& pdf) const final override;

	virtual math::float3 emission() const final override;

	virtual math::float3 attenuation() const final override;

	virtual void sample_evaluate(sampler::Sampler& sampler, BxDF_result& result) const final override;

	virtual bool is_pure_emissive() const final override;

	void set(const math::float3& color, float sqrt_roughness, float metallic);
	void set(const math::float3& color, const math::float3& emission, float sqrt_roughness, float metallic);

private:

	math::float3 diffuse_color_;
	math::float3 f0_;
	math::float3 emission_;

	float a2_;

	float metallic_;

	oren_nayar::Oren_nayar<Sample> oren_nayar_;
//	lambert::Lambert<Sample> lambert_;
	ggx::GGX<Sample> ggx_;

//	friend lambert::Lambert<Sample>;
	friend oren_nayar::Oren_nayar<Sample>;
	friend ggx::GGX<Sample>;
};

class Substitute : public Material<Sample> {
public:

	Substitute(Sample_cache<Sample>& cache, std::shared_ptr<image::texture::Texture_2D> mask);

	virtual math::float3 sample_emission(math::float2 uv, const image::texture::sampler::Sampler_2D& sampler) const override;

	virtual math::float3 average_emission() const override;

	virtual const image::texture::Texture_2D* emission_map() const override;
};

}}}
