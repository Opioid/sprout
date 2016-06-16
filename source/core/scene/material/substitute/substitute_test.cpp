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

void test() {
	math::random::Generator rng;
	sampler::Random sampler(rng, 0);

	std::cout << "substitute::testing::test()" << std::endl;

	Sample sample;

	math::float3 color(1.f, 1.f, 1.f);
	math::float3 radiance = math::float3_identity;
	float ior         = 1.47f;
	float constant_f0 = fresnel::schlick_f0(1.f, ior);
	float roughness   = 0.f;
	float metallic    = 0.f;
	sample.set(color, radiance, ior, constant_f0, roughness, metallic);

	math::float3 t(1.f, 0.f, 0.f);
	math::float3 b(0.f, 1.f, 0.f);
	math::float3 n(0.f, 0.f, 1.f);

	math::float3 arbitrary = math::normalized(math::float3(0.5f, 0.5f, 0.5f));

	math::float3 wo = n;

	sample.set_basis(t, b, n, n, wo);

	math::float3 wi = arbitrary;//n;
	float pdf;
	math::float3 reflection = sample.evaluate(wi, pdf);
	print(reflection, pdf);

	bxdf::Result result;
	sample.sample_evaluate(sampler, result);

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
}

}

}}}
