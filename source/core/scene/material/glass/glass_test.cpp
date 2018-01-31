#include "glass_test.hpp"
#include "glass_sample.hpp"
#include "glass_rough_sample.hpp"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_print.hpp"
#include "scene/material/material_sample.inl"
#include "sampler/sampler_random.hpp"
#include "base/math/vector3.inl"
#include "base/random/generator.inl"

namespace scene::material::glass::testing {

void test() {
	rnd::Generator rng;
	sampler::Random sampler(rng);
	sampler.resize(0, 1, 1, 1);

	bxdf::Sample result;

	Sample sample;

	float3 t(1.f, 0.f, 0.f);
	float3 b(0.f, 1.f, 0.f);
	float3 n(0.f, 0.f, 1.f);

	float3 wo;

	wo = math::normalize(float3(0.25f, 0.25f, -0.5f));;
	sample.set_basis(n, wo);
	sample.layer_.set_tangent_frame(t, b, n);

	float3 refraction_color(1.f, 1.f, 1.f);
	float3 absorption_color(1.f, 1.f, 1.f);
	float attenuation_distance = 1.f;
	float ior = 1.5f;

	sample.layer_.set(refraction_color, absorption_color, attenuation_distance, ior, 1.f);


	result.wi = float3::identity();
	result.reflection = float3::identity();
	Sample::BSDF::reflect(sample, sample.layer_, sampler, result);
	print(result);
	result.reflection = float3::identity();
	Sample::BSDF::refract(sample, sample.layer_, sampler, result);
	print(result);

	wo = b;
	sample.set_basis(n, wo);
	result.reflection = float3::identity();
	Sample::BSDF::reflect(sample, sample.layer_, sampler, result);
	print(result);
	result.reflection = float3::identity();
	Sample::BSDF::refract(sample, sample.layer_, sampler, result);
	print(result);

	wo = n;
	sample.set_basis(n, wo);
	result.reflection = float3::identity();
	Sample::BSDF::reflect(sample, sample.layer_, sampler, result);
	print(result);
	result.reflection = float3::identity();
	Sample::BSDF::refract(sample, sample.layer_, sampler, result);
	print(result);
}

void rough_refraction() {
	rnd::Generator rng;
	sampler::Random sampler(rng);
	sampler.resize(0, 1, 1, 1);

	bxdf::Sample result;

	Sample_rough sample;

	float3 t(1.f, 0.f, 0.f);
	float3 b(0.f, 1.f, 0.f);
	float3 n(0.f, 0.f, 1.f);

	float3 wo;

	wo = math::normalize(float3(0.25f, 0.1f, -0.5f));
	sample.set_basis(n, wo);
	sample.layer_.set_tangent_frame(t, b, n);

	float3 refraction_color(1.f, 1.f, 1.f);
	float3 absorption_color(1.f, 1.f, 1.f);
	float attenuation_distance = 1.f;
	float ior = 1.5f;
	float roughness = 0.5f;//ggx::Min_roughness;
	float alpha = roughness * roughness;

	sample.layer_.set(refraction_color, absorption_color, attenuation_distance, ior, 1.f, alpha);


	result.wi = float3::identity();
	result.reflection = float3::identity();

	float n_dot_wi = Sample_rough::BSDF::refract(sample, sample.layer_, sampler, result);
	result.reflection *= n_dot_wi;
	result.pdf *= 0.5f;
	print(result);

	if (result.pdf > 0.f) {
		bxdf::Result eval = sample.evaluate(result.wi);
		print(eval);
	}
}

}
