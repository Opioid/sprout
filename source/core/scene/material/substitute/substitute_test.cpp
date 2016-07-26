#include "substitute_test.hpp"
#include "substitute_sample.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_print.hpp"
#include "scene/material/ggx/ggx.inl"
#include "sampler/sampler_random.hpp"
#include "base/math/random/generator.inl"
#include "base/math/vector.inl"
#include "base/math/print.hpp"
#include <iostream>

namespace scene { namespace material { namespace substitute {

namespace testing {

struct Setup {
	void test(float3_p wi, float3_p wo,
			  float3_p t, float3_p b, float3_p n,
			  sampler::Sampler& sampler);

	float3 color = float3(1.f, 1.f, 1.f);
	float3 radiance = math::float3_identity;
	float ior         = 1.47f;
	float constant_f0 = fresnel::schlick_f0(1.f, ior);
	float roughness   = 0.0f;
	float metallic    = 0.f;
};

void test() {
	math::random::Generator rng;
	sampler::Random sampler(rng, 0);

	std::cout << "substitute::testing::test()" << std::endl;

	Setup setup;

	float3 t(1.f, 0.f, 0.f);
	float3 b(0.f, 1.f, 0.f);
	float3 n(0.f, 0.f, 1.f);


//	float3 t(0.f, 0.500185f, 0.865919f);
//	float3 b(-0.979966f, -0.172462f, 0.0996202f);
//	float3 n(0.199167f, -0.848824f, 0.489725f);

	float3 arbitrary = math::normalized(float3(0.5f, 0.5f, 0.5f));

//	float3 arbitrary();

	float3 wo = arbitrary;
	float3 wi = arbitrary;//n;

	setup.test(-wi, wo, t, b, n, sampler);

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
	float3 radiance = math::float3_identity;
	float ior         = 1.47f;
	float constant_f0 = fresnel::schlick_f0(1.f, ior);
	float roughness   = 0.f;
	float metallic    = 0.f;
	sample.layer_.set(color, radiance, ior, constant_f0, roughness, metallic);

	float3 t(1.f, 0.f, 0.f);
	float3 b(0.f, 1.f, 0.f);
	float3 n(0.f, 0.f, 1.f);

	float3 arbitrary = math::normalized(float3(0.5f, 0.5f, 0.5f));

	float3 wo = n;

	sample.set_basis(n, wo);
	sample.layer_.set_basis(t, b, n);

	float3 wi = arbitrary;//n;
	float pdf;
	float3 reflection = sample.evaluate(wi, pdf);
	print(reflection, pdf);

	bxdf::Result result;
	sample.sample(sampler, result);

	print(result);


	float xiy = 1.f;
	float a2 = ggx::Min_a2;
	float n_dot_h = std::sqrt((1.f - xiy) / ((a2 - 1.f) * xiy + 1.f));

	std::cout << "n_dot_h == " << n_dot_h << std::endl;

	n_dot_h = 1.f;
	float d = ggx::distribution_isotropic(n_dot_h, ggx::Min_a2);

	std::cout << "d == " << d << std::endl;

	float n_dot_wo = 1.f;
	float g = ggx::geometric_visibility(n_dot_wo, n_dot_wo, ggx::Min_a2);
	std::cout << "g == " << g << std::endl;
	*/
}

void Setup::test(float3_p wi, float3_p wo,
				 float3_p t, float3_p b, float3_p n,
				 sampler::Sampler& sampler) {
	Sample sample;

	sample.layer_.set(color, radiance, ior, constant_f0, roughness, metallic);

	sample.set_basis(n, wo);
	sample.layer_.set_basis(t, b, n);

	float pdf;
	float3 reflection = sample.evaluate(wi, pdf);
	print(reflection, pdf);

	bxdf::Result result;
	sample.sample(sampler, result);

	print(result);
}

}

}}}
