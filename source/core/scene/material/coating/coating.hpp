#pragma once

#include "scene/material/material_sample.hpp"

namespace scene::material::coating {

struct Result {
	float3 reflection;
	float3 attenuation;
	float  pdf;
};

struct Coating_base {
	void set_color_and_weight(const float3& color, float weight);

	float3 color_;
	float  weight_;
};

class Clearcoat : public Coating_base {

public:

	void set(float f0, float alpha, float alpha2);

protected:

	template<typename Layer>
	Result evaluate(f_float3 wi, f_float3 wo, const float3& h,
					float wo_dot_h, float internal_ior, const Layer& layer) const;

	template<typename Layer>
	void sample(f_float3 wo, float internal_ior,
				const Layer& layer, sampler::Sampler& sampler,
				float3& attenuation, bxdf::Sample& result) const;

public:

	float f0_;
	float alpha_;
	float alpha2_;
};

class Thinfilm : public Coating_base {

public:

	void set(float ior, float alpha, float alpha2, float thickness);

protected:

	template<typename Layer>
	Result evaluate(f_float3 wi, f_float3 wo, const float3& h,
					float wo_dot_h, float internal_ior, const Layer& layer) const;

	template<typename Layer>
	void sample(f_float3 wo, float internal_ior,
				const Layer& layer, sampler::Sampler& sampler,
				float3& attenuation, bxdf::Sample& result) const;

public:

	float ior_;
	float alpha_;
	float alpha2_;
	float thickness_;
};

template<typename Coating>
class Coating_layer : public Sample::Layer, public Coating {

public:

	Result evaluate(f_float3 wi, f_float3 wo, const float3& h,
					float wo_dot_h, float internal_ior) const;

	void sample(f_float3 wo, float internal_ior, sampler::Sampler& sampler,
				float3& attenuation, bxdf::Sample& result) const;
};

using Clearcoat_layer = Coating_layer<Clearcoat>;
using Thinfilm_layer  = Coating_layer<Thinfilm>;

}
