#ifndef SU_CORE_RENDERING_SENSOR_FILTERED_HPP
#define SU_CORE_RENDERING_SENSOR_FILTERED_HPP

#include "base/math/vector4.hpp"

namespace image::texture { class Texture; }

using Texture_ptr = std::shared_ptr<image::texture::Texture>;

namespace sampler { struct Camera_sample; }

namespace rendering::sensor {

namespace filter { class Filter; }

template<class Base, class Clamp>
class Filtered : public Base {

public:

	Filtered(int2 dimensions, float exposure, const Clamp& clamp, const filter::Filter* filter);
	Filtered(int2 dimensions, float exposure, const Texture_ptr& backplate,
			 const Clamp& clamp, const filter::Filter* filter);
	~Filtered();

	virtual int32_t filter_radius_int() const override final;

	virtual int4 isolated_tile(const int4& tile) const override final;

	virtual void add_sample(const sampler::Camera_sample& sample, const float4&,
							const int4& isolated_bounds, const int4& bounds) override final;

private:

	void add_weighted_pixel(int2 pixel, float weight, const float4& color,
							const int4& isolated_bounds, const int4& bounds);

	void weight_and_add_pixel(int2 pixel, float2 relative_offset, const float4& color,
							  const int4& isolated_bounds, const int4& bounds);

	Clamp clamp_;

	const filter::Filter* filter_;
};

}

#endif
