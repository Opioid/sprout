#pragma once

#include "light.hpp"
#include "image/texture/texture_2d.hpp"
#include "image/texture/sampler/sampler_spherical_nearest.hpp"

namespace scene { namespace light {

class Image_light : public Light {
public:

	Image_light();

	void init(std::shared_ptr<image::Image> image);

	virtual void transformation_at(float time, Composed_transformation& transformation) const final override;

	virtual void sample(const Composed_transformation& transformation, const math::float3& p, const math::float3& n,
						const image::sampler::Sampler_2D& image_sampler, sampler::Sampler& sampler,
						uint32_t max_samples, std::vector<Sample>& samples) const;

	virtual math::float3 evaluate(const math::float3& wi) const final override;

	virtual float pdf(const Composed_transformation& transformation, const math::float3& p, const math::float3& wi) const final override;

	virtual math::float3 power(const math::aabb& scene_bb) const final override;

	virtual void prepare_sampling() final override;

	virtual bool equals(const Prop* prop, uint32_t part) const final override;

private:

	image::Texture_2D texture_;
	image::sampler::Spherical_nearest sampler_nearest_;
};

}}
