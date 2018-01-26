#include "substitute_test.hpp"
#include "substitute_sample.hpp"
#include "substitute_base_sample.inl"
#include "scene/material/material_sample.inl"
#include "scene/material/material_print.hpp"
#include "scene/material/disney/disney.inl"
#include "scene/material/ggx/ggx.inl"
#include "sampler/sampler_constant.hpp"
#include "sampler/sampler_random.hpp"
#include "base/math/vector3.inl"
#include "base/math/print.hpp"
#include "base/random/generator.inl"
#include <iostream>

namespace scene::material::substitute::testing {

struct Setup {
	void test(const float3& wi, const float3& wo,
			  const float3& t, const float3& b, const float3& n,
			  sampler::Sampler& sampler);

	float3 color	  = float3(1.f);
	float3 radiance	  = float3(0.f);
	float ior         = 1.47f;
	float constant_f0 = fresnel::schlick_f0(1.f, ior);
	float roughness   = 0.0214962f;//ggx::clamp_roughness(0.0001f);
	float metallic    = 0.f;
};

void test() {
	rnd::Generator rng;
	sampler::Constant sampler(rng);
	sampler.resize(0, 1, 1, 1);
	sampler.set(float2(0.018222f, 0.626882f));
	sampler.set(1.f);

	std::cout << "substitute::testing::test()" << std::endl;

	Setup setup;

	float3 t(0.999755f, 0.015535f, -0.0157584f);
	float3 b(-0.015535f, -0.0144017f, -0.999776f);
	float3 n(-0.0157584f, 0.999776f, -0.0141569f);


//	float3 t(0.f, 0.500185f, 0.865919f);
//	float3 b(-0.979966f, -0.172462f, 0.0996202f);
//	float3 n(0.199167f, -0.848824f, 0.489725f);

//	float3 arbitrary = math::normalize(float3(0.5f, 0.5f, 0.5f));

	float3 wo = float3(0.493474f, 0.129845f, -0.860014f);
	float3 wi = float3(0.f, 1.f, 0.f);

	setup.test(wi, wo, t, b, n, sampler);




	/*
	setup.test(t, t, t, b, n, sampler);
	setup.test(t, b, t, b, n, sampler);
	setup.test(t, n, t, b, n, sampler);
	setup.test(b, t, t, b, n, sampler);
	setup.test(b, b, t, b, n, sampler);
	setup.test(b, n, t, b, n, sampler);
	setup.test(n, t, t, b, n, sampler);
	setup.test(n, b, t, b, n, sampler);
	setup.test(n, n, t, b, n, sampler);
	*/


	/*
	Sample sample;

	float3 color(1.f, 1.f, 1.f);
	float3 radiance = float3::identity();
	float ior         = 1.47f;
	float constant_f0 = fresnel::schlick_f0(1.f, ior);
	float roughness   = 0.f;
	float metallic    = 0.f;
	sample.layer_.set(color, radiance, ior, constant_f0, roughness, metallic);

	float3 t(1.f, 0.f, 0.f);
	float3 b(0.f, 1.f, 0.f);
	float3 n(0.f, 0.f, 1.f);

	float3 arbitrary = math::normalize(float3(0.5f, 0.5f, 0.5f));

	float3 wo = n;

	sample.set_tangent_frame(n, wo);
	sample.layer_.set_tangent_frame(t, b, n);

	float3 wi = arbitrary;//n;
	float pdf;
	float3 reflection = sample.evaluate(wi, pdf);
	print(reflection, pdf);

	bxdf::Sample result;
	sample.sample(sampler, result);

	print(result);


	float xiy = 1.f;
	float alpha2 = ggx::Min_alpha2;
	float n_dot_h = std::sqrt((1.f - xiy) / ((alpha2 - 1.f) * xiy + 1.f));

	std::cout << "n_dot_h == " << n_dot_h << std::endl;

	n_dot_h = 1.f;
	float d = ggx::distribution_isotropic(n_dot_h, ggx::Min_alpha2);

	std::cout << "d == " << d << std::endl;

	const float n_dot_wo = 1.f;
	float g = ggx::geometric_visibility(n_dot_wo, n_dot_wo, ggx::Min_alpha2);
	std::cout << "g == " << g << std::endl;
	*/
}

void Setup::test(const float3& wi, const float3& wo,
				 const float3& t, const float3& b, const float3& n,
				 sampler::Sampler& sampler) {
	Sample sample;

	sample.layer_.set(color, radiance, ior, constant_f0, roughness, metallic);

	std::cout << sample.layer_.alpha2_ << std::endl;

	sample.set_basis(n, wo);
	sample.layer_.set_tangent_frame(t, b, n);

	{
		const auto result = sample.evaluate(wi);
		print(result);
	}

	bxdf::Sample result;
	sample.sample(sampler, result);

	print(result);
}

}
