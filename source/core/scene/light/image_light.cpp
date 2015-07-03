#include "image_light.hpp"
#include "light_sample.hpp"
#include "sampler/sampler.hpp"
#include "image/image.hpp"
#include "image/texture/texture_2d.inl"
#include "base/math/math.hpp"
#include "base/math/sampling.hpp"
#include "base/math/vector.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace light {

Image_light::Image_light() : texture_(nullptr) {}

void Image_light::init(std::shared_ptr<image::Image> image) {
	texture_.init(image);
}

void Image_light::transformation_at(float /*time*/, entity::Composed_transformation& /*transformation*/) const {
}

void Image_light::sample(const entity::Composed_transformation& /*transformation*/,
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
	sample.energy = sampler_nearest_.sample3(texture_, dir);

	samples.push_back(sample);
}

math::float3 Image_light::evaluate(const math::float3& wi) const {
	return sampler_nearest_.sample3(texture_, wi);
}

float Image_light::pdf(const entity::Composed_transformation& /*transformation*/,
					   const math::float3& /*p*/, const math::float3& /*wi*/) const {
	return 1.f / (2.f * math::Pi);
}

math::float3 Image_light::power(const math::aabb& scene_bb) const {
	return scene_bb.volume() * (4.f * math::Pi) * texture_.image()->average().xyz;
}

void Image_light::prepare_sampling() {
}

bool Image_light::equals(const Prop* /*prop*/, uint32_t /*part*/) const {
	return false;
}

}}
