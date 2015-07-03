#include "uniform_light.hpp"
#include "light_sample.hpp"
#include "sampler/sampler.hpp"
#include "base/math/math.hpp"
#include "base/math/sampling.hpp"
#include "base/math/vector.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace light {

void Uniform_light::init(const math::float3& energy) {
	energy_ = energy;
}

void Uniform_light::transformation_at(float /*time*/, entity::Composed_transformation& /*transformation*/) const {
}

void Uniform_light::sample(const entity::Composed_transformation& /*transformation*/,
						   const math::float3& /*p*/, const math::float3& n,
                           const image::sampler::Sampler_2D& /*image_sampler*/, sampler::Sampler& sampler,
						   uint32_t /*max_samples*/, std::vector<Sample>& samples) const {
	samples.clear();

	Sample sample;

	math::float3 x, y;
	math::coordinate_system(n, x, y);

	math::float2 uv = sampler.generate_sample_2d();
	math::float3 dir = math::sample_oriented_hemisphere_uniform(uv, x, y, n);

	sample.shape.wi = dir;
	sample.shape.pdf = 1.f / (2.f * math::Pi);
	sample.shape.t = 1000.f;
	sample.energy = energy_;

	samples.push_back(sample);
}

math::float3 Uniform_light::evaluate(const math::float3& /*wi*/) const {
	return energy_;
}

float Uniform_light::pdf(const entity::Composed_transformation& /*transformation*/,
						 const math::float3& /*p*/, const math::float3& /*wi*/) const {
	return 1.f / (2.f * math::Pi);
}

math::float3 Uniform_light::power(const math::aabb& scene_bb) const {
	return scene_bb.volume() * (4.f * math::Pi) * energy_;
}

void Uniform_light::prepare_sampling() {
}

bool Uniform_light::equals(const Prop* /*prop*/, uint32_t /*part*/) const {
	return false;
}

}}
