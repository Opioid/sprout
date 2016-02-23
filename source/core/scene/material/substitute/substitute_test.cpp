#include "substitute_test.hpp"
#include "substitute_sample.hpp"
#include "scene/material/material_sample.inl"
#include "scene/material/material_print.hpp"
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

	math::vec3 color(1.f, 1.f, 1.f);
	math::vec3 emission = math::vec3_identity;
	float constant_f0 = 0.03f;
	float roughness   = 0.f;
	float metallic    = 0.f;
	float thickness   = 0.f;
	float attenuation_distance = 100.f;
	sample.set(color, emission, constant_f0, roughness, metallic, thickness, attenuation_distance);

	math::vec3 t(1.f, 0.f, 0.f);
	math::vec3 b(0.f, 1.f, 0.f);
	math::vec3 n(0.f, 0.f, 1.f);

	math::vec3 arbitrary = math::normalized(math::vec3(0.5f, 0.5f, 0.5f));

	math::vec3 wo = n;

	sample.set_basis(t, b, n, n, wo);

	math::vec3 wi = arbitrary;//n;
	float pdf;
	math::vec3 reflection = sample.evaluate(wi, pdf);
	print(reflection, pdf);

	bxdf::Result result;
	sample.sample_evaluate(sampler, result);

	print(result);
}

}

}}}
